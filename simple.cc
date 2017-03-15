#define simple_malloc(p)      malloc((p))
#define simple_calloc(n, x)   calloc((n), (x))
#define simple_realloc(p, x)  realloc((p), (x))
#define simple_free(x)        free((x))

#define GRAPHNAME_ARG 	      "graphname"
#define GRAPHNAME_ARG_LEN     9

#include <stdio.h>
#include <iostream>

#include "gcc-plugin.h"
#include "plugin.h"
#include "plugin-version.h"

#include <assert.h>

#include "tree.h"
#include "tree-ssa-alias.h"
#include "basic-block.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "tree-cfg.h"
#include "tree-pass.h"
#include "cfgloop.h"
#include "cgraph.h"
#include "options.h"
#include "dominance.h"
#include "pass_manager.h"
#include "context.h"
#include "rtl.h"
#include "tree-pass.h"
#include "tree-pretty-print.h"
#include "cfghooks.h"
#include "graph.h"
#include "gimple-iterator.h"

using namespace std;

int plugin_is_GPL_compatible = 1;

struct plugin_info simple_plugin_info
{
  .version = "0.0.1",
  .help = "A simple plugin ..."
};

/* ==========================================================
 * The simple plugin class
 */
class simple_plugin_t
{
public:
  simple_plugin_t(struct plugin_name_args *,
                  struct plugin_gcc_version *);
  virtual ~simple_plugin_t(void);

  bool has_arg(const char *);  
  const char *get_graphname(void);
  bool check_version(void);
  void dump_plugin_info(void);
  void dump_version_info(void);

  int m_argc;

protected:
  simple_plugin_t(void);
  simple_plugin_t(simple_plugin_t &);
  simple_plugin_t& operator =(simple_plugin_t &);
  simple_plugin_t *clone(simple_plugin_t *);

private:
  string m_base_name;
  string m_full_name;
  struct plugin_argument *m_argv;
  string m_version;
  string m_help;
  string m_graphname;
  struct plugin_gcc_version *m_version_infos;
  struct plugin_info *m_plugin_infos;
};

/* ==========================================================
 * A custom pass class
 */
class SimpleGimplePass : public gimple_opt_pass
{
public:
  SimpleGimplePass(const pass_data&, gcc::context *);

  bool gate (function *);
  unsigned int execute (function *);
  opt_pass *clone(void);
};

SimpleGimplePass::SimpleGimplePass(const pass_data& data,
                                   gcc::context *ctxt)
  : gimple_opt_pass(data, ctxt)
{
}

bool
SimpleGimplePass::gate(function *fun)
{
  return true;
}

unsigned int
SimpleGimplePass::execute(function *fun)
{
  return 0;
}

opt_pass *
SimpleGimplePass::clone()
{
  return this;
}

/* ==========================================================
 * Context print macro
 */

// For dumping everything define DUMP_ALL
// Warning: HUGE !!
//#define DUMP_ALL

#if DUMP_ALL

static int simple_dump_flags = 0xFFFFFFFF;

# define print_context                                                \
  printf("cfun = %p\n", cfun);                                        \
  if (cfun) {                                                         \
    printf("cfun->decl = %p\n", cfun->decl);                          \
    if (cfun->decl) {                                                 \
      print_generic_decl(stdout, cfun->decl, 0xFFFFFFFF);             \
      printf("\n");                                                   \
    }                                                                 \
    printf("cfun->gimple_body = %p\n", cfun->gimple_body);            \
    if (cfun->gimple_body) {                                          \
      print_gimple_seq(stdout, cfun->gimple_body, 4, 0xFFFFFFFF);     \
      printf("\n");                                                   \
    };                                                                \
    printf("cfun->cfg = %p\n", cfun->cfg);                            \
    if (cfun->cfg)                                                    \
    {                                                                 \
      basic_block bb;                                                 \
      gimple_dump_cfg(stdout, 0xFFFFFFFF);                            \
      FOR_ALL_BB_FN(bb, cfun)                                         \
        dump_bb (stdout, bb, 4, 0xFFFFFFFF);                          \
    }                                                                 \
  }

#else /* ! DUMP_ALL */

/*
static int simple_dump_flags = dump_flags;
*/


static int simple_dump_flags =                                        \
  TDF_BLOCKS|                                                         \
  TDF_LINENO|                                                         \
  TDF_COMMENT;

//  TDF_ASMNAME|                                                      
//  TDF_STMTADDR|                                                      
//  TDF_ADDRESS;


/*static int simple_dump_flags =                                      \
  TDF_BLOCKS|TDF_LINENO|TDF_COMMENT|                                  \
  TDF_RAW|TDF_ALIAS|TDF_EH|TDF_SLIM|                                  \
  TDF_STMTADDR|TDF_VOPS|TDF_MEMSYMS|                                  \
  TDF_UID|TDF_ASMNAME|TDF_ADDRESS;
*/

# define print_context                                                \
  printf("cfun = %p\n", cfun);                                        \
  if (cfun) {                                                         \
    printf("cfun->decl = %p\n", cfun->decl);                          \
    if (cfun->decl) {                                                 \
      print_generic_decl(stdout, cfun->decl, 0);                      \
      printf("\n");                                                   \
    }                                                                 \
    printf("cfun->cfg = %p\n", cfun->cfg);                            \
    if (cfun->cfg)                                                    \
    {                                                                 \
      basic_block bb;                                                 \
      printf("*** BASIC BLOCKS ***\n");                               \
      FOR_ALL_BB_FN(bb, cfun)                                         \
        {                                                             \
          printf("*** ========================\n");                   \
          dump_bb (stdout, bb, 4, simple_dump_flags);                 \
        }                                                             \
      if (pi->has_arg(GRAPHNAME_ARG))                                 \
        print_graph_cfg(pi->get_graphname(), cfun);                   \
    }                                                                 \
  }

#endif /* DUMP_ALL */

/* ==========================================================
 * Callbacks
 */

// on_PLUGIN_FINISH
//
// gcc_data is NULL;
void
on_PLUGIN_FINISH(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple on_PLUGIN_FINISH" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;

  print_context;
}

// on_PLUGIN_PASS_EXECUTION
//
// gcc_data is the executed pass pointer: opt_pass *
void
on_PLUGIN_PASS_EXECUTION(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple on_PLUGIN_PASS_EXECUTION" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;

  opt_pass *pass = (opt_pass *)gcc_data;
  printf("pass name = %s\n", pass->name);
  printf("pass index = %d\n", pass->static_pass_number);
  
  if (!memcmp(pass->name , "*clean_state", 12))
    {
      printf("***>>> HERE <<<***\n");

      vec<tree, va_gc> *inputs, *outputs, *clobbers, *labels;
      vec_alloc (inputs, 0);
      vec_alloc (outputs, 0);
      vec_alloc (clobbers, 0);
      vec_alloc (labels, 0);
      gasm *gimple_asm_stmt = gimple_build_asm_vec("nop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n\tnop\n",
                                                   inputs,
                                                   outputs,
                                                   clobbers,
                                                   labels);
      basic_block bb;      
      FOR_EACH_BB_FN(bb, cfun)
        {
          gimple_stmt_iterator gsi_start = gsi_start_nondebug_after_labels_bb (bb);
          gsi_insert_before_without_update (&gsi_start, gimple_asm_stmt, GSI_SAME_STMT);
          gimple_stmt_iterator gsi_last = gsi_last_nondebug_bb (bb);
          gsi_insert_after_without_update (&gsi_last, gimple_asm_stmt, GSI_SAME_STMT);
        }

      print_context;
    }
}

// on_PLUGIN_NEW_PASS
//
// gcc_data is the first executed pass pointer: opt_pass *
void
on_PLUGIN_NEW_PASS(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple PLUGIN_NEW_PASS" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;

  opt_pass *pass = (opt_pass *)gcc_data;
  printf("pass name = %s\n", pass->name);
  printf("pass index = %d\n", pass->static_pass_number);
  
  print_context;
}

// on_PLUGIN_FINISH_UNIT
//
// gcc_data is NULL
void
on_PLUGIN_FINISH_UNIT(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple plugin finished unit" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;

  print_context;
}

// on_PLUGIN_ALL_PASSES_START
//
// gcc_data is NULL
void
on_PLUGIN_ALL_PASSES_START(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple PLUGIN_ALL_PASSES_START" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;

  print_context;
}

// on_PLUGIN_ALL_PASSES_END
//
// gcc_data is NULL
void
on_PLUGIN_ALL_PASSES_END(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple PLUGIN_ALL_PASSES_END" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;

  print_context;
}

// on_PLUGIN_ALL_IPA_PASSES_START
//
// gcc_data is NULL
void
on_PLUGIN_ALL_IPA_PASSES_START(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple PLUGIN_ALL_IPA_PASSES_START" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;

  print_context;
}

// on_PLUGIN_ALL_IPA_PASSES_END
//
// gcc_data is NULL
void
on_PLUGIN_ALL_IPA_PASSES_END(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cerr << "Simple PLUGIN_ALL_IPA_PASSES_END" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;
  
  print_context;
}

// trace_callback_for_PLUGIN_EARLY_GIMPLE_PASSES_END
// 
// gcc_data is NULL
static void
trace_callback_for_PLUGIN_EARLY_GIMPLE_PASSES_END(void *gcc_data, void *user_data)
{
  cerr << "============================" << endl;
  cout << "Simple custom PLUGIN_EARLY_GIMPLE_PASSES_END" << endl;
  cout << "    gcc_data = " << gcc_data << endl;
  cout << "   user_data = " << user_data << endl;

  simple_plugin_t *pi = (simple_plugin_t *)user_data;
  
  print_context;
}

/*
 * simple_plugin_t implem
 */
 
// Constructor
simple_plugin_t::simple_plugin_t(struct plugin_name_args *args,
                                 struct plugin_gcc_version *version_infos)
{
  cout << "simple_plugin_t::simple_plugin_t(struct plugin_name_args *args): entry" << endl;

  m_graphname = "";
  m_base_name = args->base_name != NULL ? args->base_name : "";
  m_full_name = args->full_name != NULL ? args->full_name : "";
  m_argc = args->argc;

  m_argv = (struct plugin_argument *)simple_calloc(sizeof(struct plugin_argument), m_argc);
  assert(m_argv != (struct plugin_argument *)NULL);
  for (int i = 0 ; i < args->argc; i++)
    {
      m_argv[i].key = (char *)simple_malloc(strlen(args->argv[i].key));
      assert(m_argv[i].key != (char *)NULL);
      memcpy(m_argv[i].key, args->argv[i].key, strlen(args->argv[i].key)+1);

      m_argv[i].value = (char *)simple_malloc(strlen(args->argv[i].value));
      assert(m_argv[i].value != (char *)NULL);
      memcpy(m_argv[i].value, args->argv[i].value, strlen(args->argv[i].value)+1);
      
      if (!strncmp(args->argv[i].key, GRAPHNAME_ARG, GRAPHNAME_ARG_LEN))
        m_graphname = args->argv[i].value;
    }

  m_version = simple_plugin_info.version ? simple_plugin_info.version : "";
  cout << "simple_plugin_t::simple_plugin_t(struct plugin_name_args *args): m_version = '" << m_version << "'" << endl;
  m_help = simple_plugin_info.help ? simple_plugin_info.help : "";
  cout << "simple_plugin_t::simple_plugin_t(struct plugin_name_args *args): m_help = '" << m_help << "'" << endl;

  m_version_infos = version_infos;
  m_plugin_infos = &simple_plugin_info;
  
  cout << "simple_plugin_t::simple_plugin_t(struct plugin_name_args *args): exit" << endl << endl;
}
 
 
simple_plugin_t::~simple_plugin_t()
{
  cout << "simple_plugin_t::~simple_plugin_t(): entry" << endl;

  for (int i = 0; i < m_argc; i++)
    {
      if (m_argv[i].key)
        simple_free((void *)m_argv[i].key);
      if (m_argv[i].value)
        simple_free((void *)m_argv[i].value);
    }
  if (m_argv)
    simple_free((void *)m_argv);

  cout << "simple_plugin_t::~simple_plugin_t(): exit" << endl;
}

bool
simple_plugin_t::has_arg(const char *key)
{
  bool ret = false;

  cout << "bool simple_plugin_t::has_arg(const char *key): entry" << endl;

  for (int i = 0 ; i < m_argc; i++)
    if (!strncmp(m_argv[i].key, key, strlen(m_argv[i].key)))
      {
        ret = true;
        break;
      }

  cout << "bool simple_plugin_t::has_arg(const char *key): exit [" << (ret ? "true" : "false") << "]" << endl;
  return ret;
}

const char *
simple_plugin_t::get_graphname(void)
{
  return m_graphname.c_str();
}

bool
simple_plugin_t::check_version()
{
  if (!m_version_infos)
    return false;
  if (!plugin_default_version_check (m_version_infos, &gcc_version))
    {
      cerr << "This GCC plugin is for version "
           << GCCPLUGIN_VERSION_MAJOR
           << "."
           << GCCPLUGIN_VERSION_MINOR << "\n";
      return false;
    }
  return true;
}

simple_plugin_t::simple_plugin_t()
{
  cout << "simple_plugin_t::simple_plugin_t(): entry" << endl;

  m_version = simple_plugin_info.version ? simple_plugin_info.version : "";
  m_help = simple_plugin_info.help ? simple_plugin_info.help : "";

  m_version_infos = NULL;
  m_plugin_infos = &simple_plugin_info;

  cout << "simple_plugin_t::simple_plugin_t(): exit" << endl;
}

simple_plugin_t::simple_plugin_t(simple_plugin_t &plugin)
{
  cout << "simple_plugin_t::simple_plugin_t(simple_plugin_t &plugin): entry" << endl;
  cout << "simple_plugin_t::simple_plugin_t(simple_plugin_t &plugin): copy of plugin " << plugin.m_base_name << endl;

  m_base_name = plugin.m_base_name != (char *)NULL ? plugin.m_base_name : "";
  m_full_name = plugin.m_full_name != (char *)NULL ? plugin.m_full_name : "";
  m_argc = plugin.m_argc;
  m_argv = (struct plugin_argument *)simple_calloc(m_argc, sizeof(struct plugin_argument));
  assert(m_argv != (struct plugin_argument *)NULL);
  for (int i = 0 ; i < m_argc; i++)
    {
      m_argv[i].key = (char *)simple_malloc(strlen(plugin.m_argv[i].key));
      assert(m_argv[i].key != (char *)NULL);
      memcpy(m_argv[i].key, plugin.m_argv[i].key, strlen(plugin.m_argv[i].key)+1);

      m_argv[i].value = (char *)simple_malloc(strlen(plugin.m_argv[i].value));
      assert(m_argv[i].value != (char *)NULL);
      memcpy(m_argv[i].value, plugin.m_argv[i].value, strlen(plugin.m_argv[i].value)+1);

      if (!strncmp(plugin.m_argv[i].key, GRAPHNAME_ARG, GRAPHNAME_ARG_LEN))
        m_graphname = plugin.m_argv[i].value;
    }

  m_version = plugin.m_version;
  m_help = plugin.m_help;

  m_version_infos = plugin.m_version_infos;
  m_plugin_infos = plugin.m_plugin_infos;
  
  cout << "simple_plugin_t::simple_plugin_t(simple_plugin_t &plugin): exit" << endl;
}

simple_plugin_t& simple_plugin_t::operator =(simple_plugin_t &plugin)
{
  cout << "simple_plugin_t& simple_plugin_t::operator =(simple_plugin_t &plugin): entry" << endl;
  cout << "simple_plugin_t& simple_plugin_t::operator =(simple_plugin_t &plugin): assign of plugin " << plugin.m_base_name << endl;

  m_base_name = plugin.m_base_name != (char *)NULL ? plugin.m_base_name : "";
  m_full_name = plugin.m_full_name != (char *)NULL ? plugin.m_full_name : "";
  m_argc = plugin.m_argc;
  m_argv = (struct plugin_argument *)simple_calloc(plugin.m_argc, sizeof(struct plugin_argument));
  assert(m_argv != (struct plugin_argument *)NULL);
  for (int i = 0 ; i < m_argc; i++)
    {
      m_argv[i].key = (char *)simple_malloc(strlen(plugin.m_argv[i].key));
      assert(m_argv[i].key != (char *)NULL);
      memcpy(m_argv[i].key, plugin.m_argv[i].key, strlen(plugin.m_argv[i].key)+1);

      m_argv[i].value = (char *)simple_malloc(strlen(plugin.m_argv[i].value));
      assert(m_argv[i].value != (char *)NULL);
      memcpy(m_argv[i].value, plugin.m_argv[i].value, strlen(plugin.m_argv[i].value)+1);

      if (!strncmp(plugin.m_argv[i].key, GRAPHNAME_ARG, GRAPHNAME_ARG_LEN))
        m_graphname = plugin.m_argv[i].value;
    }
  m_version = plugin.m_version;
  m_help = plugin.m_help;
  
  m_version_infos = plugin.m_version_infos;
  m_plugin_infos = plugin.m_plugin_infos;
  
  cout << "simple_plugin_t& simple_plugin_t::operator =(simple_plugin_t &plugin): exit" << endl;
  return *this;
}

simple_plugin_t *simple_plugin_t::clone(simple_plugin_t *plugin_ptr)
{
  cout << "simple_plugin_t *simple_plugin_t::clone(simple_plugin_t *plugin_ptr): entry" << endl;
  cout << "simple_plugin_t *simple_plugin_t::clone(simple_plugin_t *plugin_ptr): assign of plugin " << plugin_ptr->m_base_name << endl;

  m_base_name = plugin_ptr->m_base_name != (char *)NULL ? plugin_ptr->m_base_name : "";
  m_full_name = plugin_ptr->m_full_name != (char *)NULL ? plugin_ptr->m_full_name : "";
  m_argc = plugin_ptr->m_argc;
  m_argv = (struct plugin_argument *)simple_calloc(m_argc, sizeof(struct plugin_argument));
  if (m_argv == (struct plugin_argument *)NULL)
    return (simple_plugin_t *)NULL;
  for (int i = 0 ; i < m_argc; i++)
    {
      m_argv[i].key = (char *)simple_malloc(strlen(plugin_ptr->m_argv[i].key));
      assert(m_argv[i].key != (char *)NULL);
      memcpy(m_argv[i].key, plugin_ptr->m_argv[i].key, strlen(plugin_ptr->m_argv[i].key)+1);

      m_argv[i].value = (char *)simple_malloc(strlen(plugin_ptr->m_argv[i].value));
      assert(m_argv[i].value != (char *)NULL);
      memcpy(m_argv[i].value, plugin_ptr->m_argv[i].value, strlen(plugin_ptr->m_argv[i].value)+1);

      if (!strncmp(plugin_ptr->m_argv[i].key, GRAPHNAME_ARG, GRAPHNAME_ARG_LEN))
        m_graphname = plugin_ptr->m_argv[i].value;
    }
  m_version = plugin_ptr->m_version != (char *)NULL ? plugin_ptr->m_version : "";
  m_help = plugin_ptr->m_help != (char *)NULL ? plugin_ptr->m_help : "";
  
  m_version_infos = plugin_ptr->m_version_infos;
  m_plugin_infos = plugin_ptr->m_plugin_infos;
  
  cout << "simple_plugin_t *simple_plugin_t::clone(simple_plugin_t *plugin_ptr): exit" << endl;
  return this;
}

void
simple_plugin_t::dump_plugin_info(void)
{
  cerr << "Plugin info" << endl;
  cerr << "===========" << endl << endl;
  cerr << "Base name: " << m_base_name << endl;
  cerr << "Full name: " << m_full_name << endl;
  cerr << "Arguments #:" << m_argc << endl;

  for (int i = 0; i < m_argc; i++)
    {
      cerr << "#" << i << " ARGS[" << m_argv[i].key << "] = '" << m_argv[i].value << "'" << endl; 
    }

  cerr << "Version string: " << m_version << endl;
  cerr << "Help string: " << m_help << endl;  
}

void
simple_plugin_t::dump_version_info(void)
{
  cerr << endl;
  cerr << "Version info\n";
  cerr << "============\n\n";
  cerr << "Base version: " << m_version_infos->basever << endl;
  cerr << "Date stamp: " << m_version_infos->datestamp << endl;
  cerr << "Dev phase: " << m_version_infos->devphase << endl;
  cerr << "Revision: " << m_version_infos->devphase << endl;
  cerr << "Configuration arguments: " << m_version_infos->configuration_arguments << endl;
  cerr << endl;
}  

/*
 * Register a new custom GIMPLE pass
 */
static int
SimpleGimplePass_do_init(struct plugin_name_args *plugin_info,
                         enum opt_pass_type pass_type,
                         size_t pass_sz)
{
  pass_data pass_data;
  struct opt_pass *pass;

  cerr << "SimpleGimplePass_do_init: entry" << endl;
  
  memset(&pass_data, 0, sizeof(pass_data));

  pass_data.type = pass_type;
  pass_data.name = "gimple";

#if (GCC_VERSION < 5000)
  pass_data.has_gate = true;
  pass_data.has_execute = true;
#endif

  cerr << "SimpleGimplePass_do_init: pass_data.name = " << pass_data.name << endl;
  
  switch (pass_type)
    {
    case GIMPLE_PASS:
      cerr << "SimpleGimplePass_do_init: pass_type = GIMPLE_PASS " << endl;
      pass = new SimpleGimplePass(pass_data, g);
      break;
      
    default:
      gcc_unreachable();
    }

  struct register_pass_info rpinfo;

  rpinfo.pass = pass;
  rpinfo.pos_op = PASS_POS_REPLACE;  
  rpinfo.ref_pass_instance_number = 0;
  rpinfo.reference_pass_name = "gimple";
  
  cerr << "Registering callback ... " << pass_data.name << endl;
  
  register_callback (plugin_info->base_name,
                     PLUGIN_EARLY_GIMPLE_PASSES_END,
                     trace_callback_for_PLUGIN_EARLY_GIMPLE_PASSES_END,
                     NULL);

  return 0;
}


/*
 * Plugin init function
 *
 * This is the first symbol loaded and called after loading the shared library
 */
int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  /* Create plugin instance */
  simple_plugin_t *plugin_instance = new simple_plugin_t(plugin_info, version);
  assert(plugin_instance != (simple_plugin_t *)NULL);

  /* Check that version matches */
  if (!plugin_instance->check_version ())
    return 1;

  /* Dump some infos */
  plugin_instance->dump_plugin_info();
  plugin_instance->dump_version_info();

  /* */
  SimpleGimplePass_do_init(plugin_info, GIMPLE_PASS, sizeof(struct gimple_opt_pass));

#define DEFEVENT(NAME)                                                \
  /* Register at info for the plugin: */                              \
  register_callback(plugin_info->base_name, NAME,                     \
                    on_##NAME, (void *)plugin_instance)

  DEFEVENT(PLUGIN_PASS_EXECUTION);
  /* Register at info for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_PASS_EXECUTION,
  //                    on_PLUGIN_PASS_EXECUTION, (void *)plugin_instance);

  DEFEVENT(PLUGIN_NEW_PASS);
  /* Register at info for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_NEW_PASS,
  //                    on_PLUGIN_NEW_PASS, (void *)plugin_instance);
  
  DEFEVENT(PLUGIN_FINISH);
  /* Register at-exit finalization for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_FINISH,
  //                    on_PLUGIN_FINISH, (void *)plugin_instance);
  
  DEFEVENT(PLUGIN_ALL_PASSES_START);
  /* Register before first pass from all_passes  for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_ALL_PASSES_START,
  //                    on_PLUGIN_ALL_PASSES_START, (void *)plugin_instance);
  
  DEFEVENT(PLUGIN_ALL_PASSES_END);
  /* Register after last pass from all_passes for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_ALL_PASSES_END,
  //                    on_PLUGIN_ALL_PASSES_END, (void *)plugin_instance);
  
  DEFEVENT(PLUGIN_ALL_IPA_PASSES_START);
  /* Register before first ipa pass  for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_ALL_IPA_PASSES_START,
  //                    on_PLUGIN_ALL_IPA_PASSES_START, (void *)plugin_instance);
  
  DEFEVENT(PLUGIN_ALL_IPA_PASSES_END);
  /* Register after last ipa pass for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_ALL_IPA_PASSES_END,
  //                    on_PLUGIN_ALL_IPA_PASSES_END, (void *)plugin_instance);
  
  DEFEVENT(PLUGIN_FINISH_UNIT);
  /* Register at-exit finalization for the plugin: */
  //  register_callback(plugin_info->base_name, PLUGIN_FINISH_UNIT,
  //                    on_PLUGIN_FINISH_UNIT, (void *)plugin_instance);
#undef DEFEVENT
  
  cerr << "Plugin successfully initialized\n";
  
  return 0;  
}

  
