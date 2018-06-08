/***************************************************************************
 *  Environment.cpp - Environment Impl
 *
 *  Created: 2018-06-04 16:18:54
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Environment.h"
#include "Log.h"

extern "C" {
#include "clips.h"
};

namespace CLIPS {

std::map<void*, Environment*> Environment::m_environment_map;

void Environment::s_clear_callback(void *env)
{
    /* LOGI("Call s_ClearCallBack(%p) this[%p]\n", env, m_environment_map[env]); */
    Environment *context = m_environment_map[env];
    if (context->m_clear_cb) {
        context->m_clear_cb();
    }
}

void Environment::s_periodic_callback(void *env)
{
    /* LOGI("Call s_periodic_callback(%p) this[%p]\n", env, m_environment_map[env]); */
    Environment *context = m_environment_map[env];
    if (context->m_periodic_cb) {
        context->m_periodic_cb();
    }
}

void Environment::s_reset_callback(void *env)
{
    /* LOGI("Call s_reset_callback(%p) this[%p]\n", env, m_environment_map[env]); */
    Environment *context = m_environment_map[env];
    if (context->m_reset_callback) {
        context->m_reset_callback();
    }
}

void Environment::s_rulefiring_callback(void *env)
{
    /* LOGI("Call s_rulefiring_callback(%p) this[%p]\n", env, m_environment_map[env]); */
    Environment *context = m_environment_map[env];
    if (context->m_rulefiring_cb) {
        context->m_rulefiring_cb();
    }
}

Environment::Environment()
#ifndef DEBUG_MEMORY
    : ClipsObject(0)
#else
    : ClipsObject("Environment", 0)
#endif
    , m_clear_cb(0), m_periodic_cb(0), m_reset_callback(0), m_rulefiring_cb(0)
{
    LOGD("Environment construct.\n");

    m_cobj = CreateEnvironment();
    m_environment_map[m_cobj] = this;

    EnvAddClearFunction(m_cobj, "clear-callback", Environment::s_clear_callback, 2001);
    EnvAddPeriodicFunction(m_cobj, "periodic-callback", Environment::s_periodic_callback, 2001);
    EnvAddResetFunction(m_cobj, "reset-callback", Environment::s_reset_callback, 2001);
    EnvAddRunFunction(m_cobj, "run-callback", Environment::s_rulefiring_callback, 2001);
}

Environment::~Environment()
{
    LOGD("Environment destruct.\n");

    EnvRemoveClearFunction(m_cobj, "clear-callback");
    EnvRemovePeriodicFunction(m_cobj, "periodic-callback");
    EnvRemoveResetFunction(m_cobj, "reset-callback");
    EnvRemoveRunFunction(m_cobj, "run-callback");

    m_environment_map.erase(m_cobj);

    DestroyEnvironment(m_cobj);

    std::map<std::string, char *>::iterator r;
    for (r = m_func_restr.begin(); r != m_func_restr.end(); ++r)
        free(r->second);
    m_func_restr.clear();
}

/*{{{ general --> */
int Environment::load(const std::string& filename)
{
    return EnvLoad(m_cobj, filename.c_str());
}

bool Environment::build(const std::string& construct)
{
    return EnvBuild(m_cobj, construct.c_str());
}

void Environment::reset()
{
    EnvReset(m_cobj);
}

bool Environment::save(const std::string &filename)
{
    return EnvSave(m_cobj, filename.c_str());
}

bool Environment::binary_load(const std::string &filename)
{
    return EnvBload(m_cobj, filename.c_str());
}

bool Environment::binary_save(const std::string &filename)
{
    return EnvBsave(m_cobj, filename.c_str());
}

Values Environment::evaluate(const std::string& expression)
{
    DATA_OBJECT clipsdo;
    int result = EnvEval(m_cobj, expression.c_str(), &clipsdo);
    if (result)
        return data_object_to_values(&clipsdo);
    return Values();
}

bool Environment::batch_evaluate(const std::string& filename)
{
    return EnvBatchStar(m_cobj, filename.c_str());
}

Values Environment::function(const std::string &function_name, const std::string &arguments)
{
    DATA_OBJECT clipsdo;
    int result = EnvFunctionCall(m_cobj, function_name.c_str(), arguments.c_str(), &clipsdo);
    if (!result)
        return data_object_to_values(&clipsdo);
    return Values();
}
/* <-- general }}}*/

/*{{{ debug --> */
int Environment::is_watched(const std::string &item)
{
    return EnvGetWatchItem(m_cobj, item.c_str());
}

bool Environment::watch(const std::string &item)
{
    /*
     * item:
     *      facts, rules, activations, focus, compilations,
     *      statistics, globals, instances, slots, messages, message-handlers,
     *      generic-functions, method, or deffunctions
     */
    return EnvWatch(m_cobj, item.c_str());
}

bool Environment::unwatch(const std::string &item)
{
    return EnvUnwatch(m_cobj, item.c_str());
}

bool Environment::is_dribble_active()
{
    return EnvDribbleActive(m_cobj);
}

bool Environment::dribble_on(const std::string &filename)
{
    return EnvDribbleOn(m_cobj, filename.c_str());
}

bool Environment::dribble_off()
{
    return EnvDribbleOff(m_cobj);
}
/* <-- debug }}}*/

/*{{{ run --> */
long int Environment::run(long int runlimit)
{
    return EnvRun(m_cobj, runlimit);
}

void Environment::refresh_agenda()
{
    EnvRefreshAgenda(m_cobj, 0);
}

void Environment::refresh_agenda(const Module &module)
{
    if (module.cobj())
        EnvRefreshAgenda(m_cobj, module.cobj());
}

void Environment::refresh_agenda(Module::pointer module)
{
    if (module && module->cobj())
        EnvRefreshAgenda(m_cobj, module->cobj());
}

void Environment::reorder_agenda()
{
    EnvReorderAgenda(m_cobj, 0);
}

void Environment::reorder_agenda(const Module &module)
{
    if (module.cobj())
        EnvReorderAgenda(m_cobj, module.cobj());
}

void Environment::reorder_agenda(Module::pointer module)
{
    if (module && module->cobj())
        EnvReorderAgenda(m_cobj, module->cobj());
}
/* <-- run }}}*/

/*{{{ facts --> */
Fact::pointer Environment::get_facts()
{
    void *fact = EnvGetNextFact(m_cobj, 0);
    if (fact)
        return Fact::create(*this, fact);
    return Fact::pointer();
}

Fact::pointer Environment::assert_fact(const std::string &factString)
{
    void* fact = EnvAssertString(m_cobj, factString.c_str());
    if (fact)
        return Fact::create(*this, fact);
    return Fact::pointer();
}

DefaultFacts::pointer Environment::get_default_facts(const std::string &default_facts_name)
{
    void* deffacts = EnvFindDeffacts(m_cobj, default_facts_name.c_str());
    if (deffacts)
        return DefaultFacts::create(*this, deffacts);
    return DefaultFacts::pointer();
}

std::vector< std::string > Environment::get_default_facts_names()
{
    DATA_OBJECT clipsdo;
    EnvGetDeffactsList(m_cobj, &clipsdo, 0);
    return data_object_to_strings(&clipsdo);
}

std::vector<std::string> Environment::get_default_facts_names(const Module &module)
{
    DATA_OBJECT clipsdo;
    if (module.cobj()) {
        EnvGetDeffactsList(m_cobj, &clipsdo, module.cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}

std::vector<std::string> Environment::get_default_facts_names(Module::pointer module)
{
    DATA_OBJECT clipsdo;
    if (module && module->cobj()) {
        EnvGetDeffactsList(m_cobj, &clipsdo, module->cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}

DefaultFacts::pointer Environment::get_default_facts_list_head()
{
    void *df = EnvGetNextDeffacts(m_cobj, 0);
    if (df)
        return DefaultFacts::create(*this, df);
    return DefaultFacts::pointer();
}
/* <-- defaultfacts }}}*/

/*{{{ module --> */
Module::pointer Environment::get_module(const std::string &module_name)
{
    void *module = EnvFindDefmodule(m_cobj, module_name.c_str());
    if (module)
        return Module::create(*this, module);
    return Module::pointer();
}

Module::pointer Environment::get_module_list_head()
{
    void *module = EnvGetNextDefmodule(m_cobj, 0);
    if (module)
        return Module::create(*this, module);
    return Module::pointer();
}

std::vector<std::string> Environment::get_module_names()
{
    DATA_OBJECT clipsdo;
    EnvGetDefmoduleList(m_cobj, &clipsdo);
    return data_object_to_strings(&clipsdo);
}

Module::pointer Environment::get_current_module()
{
    void *module = EnvGetCurrentModule(m_cobj);
    if (module)
        return Module::create(*this, module);
    return Module::pointer();
}

Module::pointer Environment::get_focused_module()
{
    void *module = EnvGetFocus(m_cobj);
    if (module)
        return Module::create(*this, module);
    return Module::pointer();
}

std::vector<std::string> Environment::get_focus_stack()
{
    DATA_OBJECT clipsdo;
    EnvGetFocusStack(m_cobj, &clipsdo);
    return data_object_to_strings(&clipsdo);
}
/* <-- module }}}*/

/*{{{ global --> */
Global::pointer Environment::get_global(const std::string &global_name)
{
    void *clips_global = EnvFindDefglobal(m_cobj, global_name.c_str());
    if (clips_global)
        return Global::create(*this, clips_global);
    return Global::pointer();
}

Global::pointer Environment::get_global_list_head()
{
    void *global = EnvGetNextDefglobal(m_cobj, 0);
    if (global)
        return Global::create(*this, global);
    return Global::pointer();
}

std::vector<std::string> Environment::get_globals_names()
{
    DATA_OBJECT clipsdo;
    EnvGetDefglobalList(m_cobj, &clipsdo, 0);
    return data_object_to_strings(&clipsdo);
}

std::vector<std::string> Environment::get_globals_names(const Module &module)
{
    DATA_OBJECT clipsdo;
    if (module.cobj()) {
        EnvGetDefglobalList(m_cobj, &clipsdo, module.cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}

std::vector<std::string> Environment::get_globals_names(Module::pointer module)
{
    DATA_OBJECT clipsdo;
    if (module && module->cobj()) {
        EnvGetDefglobalList(m_cobj, &clipsdo, module->cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}
/* <-- global }}}*/

/*{{{ function --> */
Function::pointer Environment::get_function(const std::string &function_name)
{
    void *function = EnvFindDeffunction(m_cobj, function_name.c_str());
    if (function)
        return Function::create(*this, function);
    return Function::pointer();
}

Function::pointer Environment::get_function_list_head()
{
    void *function = EnvGetNextDeffunction(m_cobj, 0);
    if (function)
        return Function::create(*this, function);
    return Function::pointer();
}

std::vector<std::string> Environment::get_function_names()
{
    DATA_OBJECT clipsdo;
    EnvGetDeffunctionList(m_cobj, &clipsdo, 0);
    return data_object_to_strings(&clipsdo);
}

std::vector<std::string> Environment::get_function_names(const Module &module)
{
    DATA_OBJECT clipsdo;
    if (module.cobj()) {
        EnvGetDeffunctionList(m_cobj, &clipsdo, (defmodule*)(module.cobj()));
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}

std::vector<std::string> Environment::get_function_names(Module::pointer module)
{
    DATA_OBJECT clipsdo;
    if (module && module->cobj()) {
        EnvGetDeffunctionList(m_cobj, &clipsdo, (defmodule*)(module->cobj()));
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}
/* <-- function }}}*/

/*{{{ template --> */
Template::pointer Environment::get_template(const std::string &template_name)
{
    if (m_cobj) {
        void *temp = EnvFindDeftemplate(m_cobj, template_name.c_str());
        if (temp)
            return Template::create(*this, temp);
    }
    return Template::pointer();
}

Template::pointer Environment::get_template_list_head()
{
    void *tem = EnvGetNextDeftemplate(m_cobj, 0);
    if (tem)
        return Template::create(*this, tem);
    return Template::pointer();
}

std::vector< std::string > Environment::get_template_names()
{
    DATA_OBJECT clipsdo;
    EnvGetDeftemplateList(m_cobj, &clipsdo, 0);
    return data_object_to_strings(&clipsdo);
}

std::vector<std::string> Environment::get_template_names(const Module &module)
{
    DATA_OBJECT clipsdo;
    if (module.cobj()) {
        EnvGetDeftemplateList(m_cobj, &clipsdo, module.cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}

std::vector<std::string> Environment::get_template_names(Module::pointer module)
{
    DATA_OBJECT clipsdo;
    if (module && module->cobj()) {
        EnvGetDeftemplateList(m_cobj, &clipsdo, module->cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}
/* <-- template }}}*/

/*{{{ rule --> */
Rule::pointer Environment::get_rule(const std::string &rule_name)
{
    void *rule = EnvFindDefrule(m_cobj, rule_name.c_str());
    if (rule)
        return Rule::create(*this, rule);
    return Rule::pointer();
}

Rule::pointer Environment::get_rule_list_head()
{
    void *rule = EnvGetNextDefrule(m_cobj, 0);
    if (rule)
        return Rule::create(*this, rule);
    return Rule::pointer();
}

std::vector< std::string > Environment::get_rule_names()
{
    DATA_OBJECT clipsdo;
    EnvGetDefruleList(m_cobj, &clipsdo, 0);
    return data_object_to_strings(&clipsdo);
}

std::vector<std::string> Environment::get_rule_names(const Module &module)
{
    DATA_OBJECT clipsdo;
    if (module.cobj()) {
        EnvGetDefruleList(m_cobj, &clipsdo, module.cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}

std::vector<std::string> Environment::get_rule_names(Module::pointer module)
{
    DATA_OBJECT clipsdo;
    if (module && module->cobj()) {
        EnvGetDefruleList(m_cobj, &clipsdo, module->cobj());
        return data_object_to_strings(&clipsdo);
    }
    return std::vector<std::string>();
}

void Environment::remove_rules()
{
    EnvUndefrule(m_cobj, 0);
}
/* <-- rule }}}*/

/*{{{ add_function --> */
bool Environment::remove_function(std::string name)
{
    bool result = UndefineFunction(m_cobj, name.c_str());
    m_functors.erase(name);
    if (m_func_restr.find(name) != m_func_restr.end()) {
        free(m_func_restr[name]);
        m_func_restr.erase(name);
    }
    return result;
}

int Environment::get_arg_count(void *env)
{
    return EnvRtnArgCount(env);
}

void* Environment::get_function_context(void *env)
{
    return GetEnvironmentFunctionContext(env);
}

void Environment::set_return_values(void *env, void *rv, const Values &v)
{
    void *mfptr = EnvCreateMultifield(env, v.size());
    for (unsigned int i = 0; i < v.size(); ++i) {
        unsigned int mfi = i + 1;
        switch (v[i].type()) {
        case TYPE_FLOAT:
            SetMFType(mfptr, mfi, FLOAT);
            SetMFValue(mfptr, mfi, EnvAddDouble(env, v[i].as_float()));
            break;
        case TYPE_INTEGER:
            SetMFType(mfptr, mfi, INTEGER);
            SetMFValue(mfptr, mfi, EnvAddLong(env, v[i].as_integer()));
            break;
        case TYPE_SYMBOL:
            SetMFType(mfptr, mfi, SYMBOL);
            SetMFValue(mfptr, mfi,
                EnvAddSymbol(env, v[i].as_string().c_str()));
            break;
        case TYPE_STRING:
            SetMFType(mfptr, mfi, STRING);
            SetMFValue(mfptr, mfi,
                EnvAddSymbol(env, v[i].as_string().c_str()));
            break;
        case TYPE_INSTANCE_NAME:
            SetMFType(mfptr, mfi, INSTANCE_NAME);
            SetMFValue(mfptr, mfi,
                EnvAddSymbol(env, v[i].as_string().c_str()));
            break;
        case TYPE_EXTERNAL_ADDRESS:
            SetMFType(mfptr, mfi, EXTERNAL_ADDRESS);
            SetMFValue(mfptr, mfi,
                EnvAddExternalAddress(env, (char*)v[i].as_string().c_str(), EXTERNAL_ADDRESS));
            break;
        default:
            throw std::logic_error("clipsmm: value type not supported for multifield return value");
        }
    }

    DATA_OBJECT_PTR rvptr = static_cast<DATA_OBJECT_PTR>(rv);

    SetpType(rvptr, MULTIFIELD);
    SetpValue(rvptr, mfptr);

    SetpDOBegin(rvptr, 1);
    SetpDOEnd(rvptr, v.size());
}

void* Environment::add_symbol(void *env, const char *s)
{
    return EnvAddSymbol(env, s);
}
/* <-- add_function }}}*/

} /* namespace CLIPS */
