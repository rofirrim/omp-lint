#include <iostream>
#include <cassert>
#include <set>
#include <map>
#include <utility>
#include <algorithm>
#include <iterator>

// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"

#include "cp/cp-tree.h"
#include "context.h"
#include "function.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "basic-block.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-ssa.h"
#include "tree-pretty-print.h"
#include "tree-pass.h"
#include "tree-ssa-operands.h"
#include "tree-phinodes.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "diagnostic.h"
#include "stringpool.h"

#include "ssa-iterators.h"

// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

static struct plugin_info my_gcc_plugin_info = { "1.0", "This plugin diagnoses dubious usage of OpenMP constructs" };

namespace {

const pass_data omp_lint_data = 
{
    GIMPLE_PASS,
    "omp_lint", /* name */
    OPTGROUP_NONE,             /* optinfo_flags */
    TV_NONE,                   /* tv_id */
    PROP_gimple_any,           /* properties_required */
    0,                         /* properties_provided */
    0,                         /* properties_destroyed */
    0,                         /* todo_flags_start */
    0                          /* todo_flags_finish */
};

struct omp_lint : gimple_opt_pass
{
    omp_lint(gcc::context *ctx)
        : gimple_opt_pass(omp_lint_data, ctx)
    {
    }

    virtual unsigned int execute(function *fun) override
    {
        warn_captured_dependences(fun);

        return 0;
    }

    virtual omp_lint* clone() override
    {
        // We do not clone ourselves
        return this;
    }

    private:
        void warn_captured_dependences(function *fun);
        void process_omp_task_clauses(tree clauses);
};


void omp_lint::warn_captured_dependences(function *fun)
{
    basic_block bb;
    FOR_ALL_BB_FN(bb, fun)
    {
#if 0
        fprintf(stderr, "--------------\n");
        gimple_bb_info *bb_info = &bb->il.gimple;
        print_gimple_seq(stderr, bb_info->seq, 0, 0);
        fprintf(stderr, "- ------------\n");
#endif

        gimple_stmt_iterator gsi;
        for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi))
        {
            gimple stmt = gsi_stmt (gsi);

            switch (gimple_code(stmt))
            {
                case GIMPLE_OMP_TASK:
                    {
                        tree clauses = gimple_omp_task_clauses(stmt);
                        process_omp_task_clauses(clauses);
                        break;
                    }
                default: ;
            }
        }
    }
}

void omp_lint::process_omp_task_clauses(tree clauses)
{
    std::set<tree> privated_vars;
    std::set<tree> firstprivated_vars;
    std::set<tree> dependent_vars;
    std::map<tree, location_t> dependent_vars_loc;

    for (tree it = clauses; it != NULL; it = OMP_CLAUSE_CHAIN(it))
    {
        switch (OMP_CLAUSE_CODE(it))
        {
            case OMP_CLAUSE_PRIVATE:
                {
                    tree decl = OMP_CLAUSE_DECL(it);
                    gcc_assert(TREE_CODE(decl) == VAR_DECL
                            || TREE_CODE(decl) == PARM_DECL);
                    privated_vars.insert(decl);
                    break;
                }
            case OMP_CLAUSE_FIRSTPRIVATE:
                {
                    tree decl = OMP_CLAUSE_DECL(it);
                    gcc_assert(TREE_CODE(decl) == VAR_DECL
                            || TREE_CODE(decl) == PARM_DECL);
                    firstprivated_vars.insert(decl);
                    break;
                }
            case OMP_CLAUSE_DEPEND:
                {
                    tree dep_expr = OMP_CLAUSE_DECL(it);

                    if (TREE_CODE(dep_expr) == ADDR_EXPR)
                    {
                        tree referenced = TREE_OPERAND(dep_expr, 0);
                        if (TREE_CODE(referenced) == VAR_DECL
                                || TREE_CODE(referenced) == PARM_DECL)
                        {
                            dependent_vars.insert(referenced);
                            dependent_vars_loc[referenced] = OMP_CLAUSE_LOCATION(it);
                        }
                        else
                        {
                            // array sections
                        }
                    }

                    break;
                }
            default:
                break;
        }
    }

    std::set<tree> privated_deps;
    std::set_intersection(
            privated_vars.begin(), privated_vars.end(),
            dependent_vars.begin(), dependent_vars.end(),
            std::inserter(privated_deps, privated_deps.begin()));
    for (auto &var_decl: privated_deps)
    {
        warning_at(dependent_vars_loc[var_decl], 0,
                "variable %qD appears in a dependence but it has a private data-sharing", var_decl);
        inform(dependent_vars_loc[var_decl],
                "the task will be scheduled according to the dependence of %qD "
                "but the task will use an uninitialized private copy",
                var_decl);
    }

    privated_deps.clear();
    std::set_intersection(
            firstprivated_vars.begin(), firstprivated_vars.end(),
            dependent_vars.begin(), dependent_vars.end(),
            std::inserter(privated_deps, privated_deps.begin()));
    for (auto &var_decl: privated_deps)
    {
        warning_at(dependent_vars_loc[var_decl], 0,
                "variable %qD appears in a dependence but has a firstprivate data-sharing", var_decl);
        inform(dependent_vars_loc[var_decl],
                "the task will be scheduled according to the dependence of %qD "
                "but the task will use a private copy initialized with the value that %qD had when the task was created",
                var_decl, var_decl);
    }
}

}

int plugin_init (struct plugin_name_args *plugin_info,
    struct plugin_gcc_version *version)
{
    // We check the current gcc loading this plugin against the gcc we used to
    // created this plugin
    if (!plugin_default_version_check (version, &gcc_version))
    {
        std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
        return 1;
    }

    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_INFO,
            /* callback */ NULL, /* user_data */ &my_gcc_plugin_info);

    // Register the phase right after cfg
    struct register_pass_info pass_info;

    pass_info.pass = new omp_lint(g);
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);

    return 0;
}

