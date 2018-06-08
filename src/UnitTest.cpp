/***************************************************************************
 *  UnitTest.cpp - UnitTest
 *
 *  Created: 2018-06-04 18:38:14
 *
 *  Copyright QRS
 ****************************************************************************/

#include "Environment.h"
#include "Log.h"

extern "C"
#include "clips.h"

#include <unistd.h>
#include <sys/time.h>
#include <iostream>
#include <sstream>

using namespace CLIPS;

#define TEST_VERSION_MAJOR 0
#define TEST_VERSION_MINOR 0
#define TEST_VERSION_MICRO 1

#define SHOW_VALUE(item, tok) \
    do { \
        switch (item.type()) { \
        case TYPE_FLOAT: \
            LOGD(tok"value  = float(%f)\n", item.as_float()); break; \
        case TYPE_INTEGER: \
            LOGD(tok"value = int(%d)\n", item.as_integer()); break; \
        case TYPE_SYMBOL: \
        case TYPE_STRING: \
        case TYPE_INSTANCE_NAME: \
            LOGD(tok"value = string(%s)\n", item.as_string().c_str()); break; \
        case TYPE_EXTERNAL_ADDRESS: \
        case TYPE_INSTANCE_ADDRESS: \
            LOGD(tok"value = address(TODO)\n"); \
        } \
    } while(0)

#define SHOW_VALUES(items, tok) \
    do { \
        for (unsigned int i = 0; i < items.size(); ++i) \
            SHOW_VALUE(items[i], tok); \
    } while(0)

class Test {/*{{{*/
public:
    void onClear(void)
    {
        /* LOGD("onClear this[%p]\n", this); */
    }
    void onPeriodic(void)
    {
        /* LOGD("onPeriodic this[%p]\n", this); */
    }
    void onReset(void)
    {
        /* LOGD("onReset this[%p]\n", this); */
    }
    void onRuleFiring(void)
    {
        /* LOGD("onRuleFiring this[%p]\n", this); */
    }

    Values clipsCallNow()
    {
        LOGD("clips call call now\n");
        Values rv;
        struct timeval tv;
        gettimeofday(&tv, 0);
        rv.push_back(tv.tv_sec);
        rv.push_back(tv.tv_usec);
        return rv;
    }

    void clipsCallTellVersion(std::string ver)
    {
        LOGD("clips calll tell version: %s\n", ver.c_str());
    }

    std::string clipsCallGetConfDir()
    {
        LOGD("clips call get-conf-dir\n");
        return "test";
    }

    Values clipsCallGetClipsDirs()
    {
        LOGD("clips call get-clips-dirs\n");
        Values rv;
        rv.push_back((char*)"rules");
        rv.push_back((char*)"classes");
        return rv;
    }

};/*}}}*/

Test gTest;

void setupClips(Environment *env)
{/*{{{*/
    bool ret = false;
    init_clips_logger(env->cobj());

    /*
     * Test Build
     */
    char buff[1024] = { 0 };
    snprintf(buff, 1023,
        "(defglobal\n"
        "  ?*VERSION-MAJOR* = %u\n"
        "  ?*VERSION-MINOR* = %u\n"
        "  ?*VERSION-MICRO* = %u\n"
        ")\n",
        TEST_VERSION_MAJOR,
        TEST_VERSION_MINOR,
        TEST_VERSION_MICRO);
    env->build(buff);

    ret = env->add_function("now", std::make_shared<Functor<Values>>(&gTest, &Test::clipsCallNow));
    ret = env->add_function("tell-ver", std::make_shared<Functor<void, std::string>>(&gTest, &Test::clipsCallTellVersion));
    ret = env->add_function("get-conf-dir", std::make_shared<Functor<std::string>>(&gTest, &Test::clipsCallGetConfDir));
    ret = env->add_function("get-clips-dirs", std::make_shared<Functor<Values>>(&gTest, &Test::clipsCallGetClipsDirs));
    LOGD("add_function ret = %d\n", ret);
}/*}}}*/

void startClips(Environment *env)
{/*{{{*/
    int ret = -1;

    ret = env->batch_evaluate("test/init.clp");
    LOGD("batch_evaluat: %d\n", ret);

    env->reset();

    Fact::pointer f = env->assert_fact("(init)");
}/*}}}*/

void test_logger(Environment *env)
{/*{{{*/
    /*
     *  测试printout stdout/debug/error/warn/info
     */
    env->evaluate("(printout stdout \"stdout-printf\" crlf)");
    env->evaluate("(printout debug \"debug-printf\" crlf)");
    env->evaluate("(printout error \"error-printf\" crlf)");
    env->evaluate("(printout warn \"warning-printf\" crlf)");
    env->evaluate("(printout info \"info-printf\" crlf)");
}/*}}}*/

void test_debug(Environment *env)
{/*{{{*/
#define IS_WATCHED(item) do { LOGD("is_watched(%s) = %d\n", item, env->is_watched(item)); } while (0)
#define WATCHED(item) do { LOGD("watch(%s)\n", item); env->watch(item); } while (0)
#define UNWATCHED(item) do { LOGD("unwatch(%s)\n", item); env->unwatch(item); } while (0)

    /*
     * Watch
     */
    IS_WATCHED("globals");
    IS_WATCHED("facts");
    IS_WATCHED("rules");
    IS_WATCHED("deffunctions");
    IS_WATCHED("instances");
    IS_WATCHED("messages");
    IS_WATCHED("message-handlers");

    WATCHED("globals");
    WATCHED("facts");
    WATCHED("rules");
    WATCHED("deffunctions");
    WATCHED("instances");
    WATCHED("messages");
    WATCHED("message-handlers");

    IS_WATCHED("globals");
    IS_WATCHED("facts");
    IS_WATCHED("rules");
    IS_WATCHED("deffunctions");
    IS_WATCHED("instances");
    IS_WATCHED("messages");
    IS_WATCHED("message-handlers");

    UNWATCHED("deffunctions");
    UNWATCHED("message-handlers");

    IS_WATCHED("deffunctions");
    IS_WATCHED("message-handlers");

    /*
     * Dribble: clips log to file
     */
    LOGD("dribble_on: %d\n", env->dribble_on("/tmp/clips.log"));
    env->reset();
    LOGD("dribble_off: %d\n", env->dribble_off());

#undef UNWATCHED
#undef WATCHED
#undef IS_WATCHED
}/*}}}*/

void test_add_function(Environment *env)
{/*{{{*/
    /*
     * 测试add_function: <返回值:Mult, 参数: 无>
     */
    env->evaluate("(now)");

    /*
     * 测试add_function: <返回值:任意(排除Mult,String), 参数:任意一个>
     */
    env->evaluate("(tell-ver \"1.0.0\")");

    /*
     * 测试add_function: <返回值: String, 参数: 无>
     */
    env->evaluate("(printout debug (get-conf-dir) crlf)");
}/*}}}*/

void test_module(Environment *env)
{/*{{{*/
    std::vector<std::string> names = env->get_module_names();
    for (unsigned int i = 0; i < names.size(); ++i)
        LOGD("name[%d] = %s\n", i, names[i].c_str());
    Module::pointer module = env->get_current_module();
    if (module) {
        LOGD("current module: %s\n", module->name().c_str());
        LOGD("\tformat: \n%s\n", module->formatted().length() ? module->formatted().c_str() : "null");
        LOGD("\tnext: %s\n", module->next() ? module->next()->name().c_str() : "null");
        LOGD("\tforcused: %s\n", env->get_focused_module()->name().c_str());
    }
}/*}}}*/

void test_global(Environment *env)
{/*{{{*/
    Global::pointer global = 0;
    std::vector<std::string> names = env->get_globals_names();
    for (unsigned int i = 0; i < names.size(); ++i) {
        global = env->get_global(names[i]);
        if (global) {
            LOGD("global name: %s, is_watched: %d\n", names[i].c_str(), global->is_watched());
            Values values = global->value();
            for (unsigned int j = 0; j < values[j].size(); ++j)
                SHOW_VALUE(values[j], "\t");
        }
    }
}/*}}}*/

void test_function(Environment *env)
{/*{{{*/
    Function::pointer function = 0;
    std::vector<std::string> names = env->get_function_names();
    for (unsigned int i = 0; i < names.size(); ++i) {
        function = env->get_function(names[i]);
        if (function) {
            LOGD("function name: %s, is_watched: %d\n", names[i].c_str(), function->is_watched());
            if (function->formatted().length()) {
                LOGD("format:\n%s\n", function->formatted().c_str());
            }
        }
    }
}/*}}}*/

void test_template(Environment *env)
{/*{{{*/
    Template::pointer temp = 0;
    std::vector<std::string> names = env->get_template_names();
    for (unsigned int i = 0; i < names.size(); ++i) {
        temp = env->get_template(names[i]);
        if (temp) {
            LOGD("template name: %s, is_watched: %d\n", names[i].c_str(), temp->is_watched());
            if (temp->formatted().length()) {
                LOGD("format: \n%s\n", temp->formatted().c_str());
            }
        }
        std::vector<std::string> slots = temp->slot_names();
        for (unsigned int j = 0; j < slots.size(); ++j) {
           LOGD("slot_name[%s] default_type[%d] is_single[%d] is_multi[%d]\n", \
               slots[j].c_str(), \
               temp->slot_default_type(slots[j]), \
               temp->is_single_field_slot(slots[j]),  \
               temp->is_multifield_slot(slots[j]));
           Values allowedValues = temp->slot_allowed_values(slots[j]);
           LOGD("\tallowedValues: size = %d\n", allowedValues.size());
           SHOW_VALUES(allowedValues, "\t\t");
           Values rangeValues = temp->slot_range(slots[j]);
           LOGD("\trangeValues: size = %d\n", rangeValues.size());
           SHOW_VALUES(rangeValues, "\t\t");
           Values defaultValues = temp->slot_default_value(slots[j]);
           LOGD("\tdefaultValues: size = %d\n", defaultValues.size());
           SHOW_VALUES(defaultValues, "\t\t");
        }
    }
}/*}}}*/

void test_rule(Environment *env)
{/*{{{*/
    Rule::pointer rule = 0;
    std::vector<std::string> names = env->get_rule_names();
    for (unsigned int i = 0; i < names.size(); ++i) {
        rule = env->get_rule(names[i]);
        if (rule) {
            if (rule->formatted().length()) {
                LOGD("\tformat:\n%s\n", rule->formatted().c_str());
            }
            LOGD("rule_name: %s, activation_watch: %d, firing_watch: %d\n", \
                names[i].c_str(), \
                rule->activations_watched(), \
                rule->firings_watched());
        }
    }
}/*}}}*/

int main(int argc, char *argv[])
{/*{{{*/
    int ret = -1;

    LOGD("\n\n********************Main start****************\n\n");

    Environment *env = new Environment();

    /*
     * Regist clear,reset,periodic,run callback
     */
    env->regist_clear_callback(std::bind(&Test::onClear, &gTest));
    env->regist_periodic_callback(std::bind(&Test::onPeriodic, &gTest));
    env->regist_reset_callback(std::bind(&Test::onReset, &gTest));
    env->regist_rulefiring_callback(std::bind(&Test::onRuleFiring, &gTest));

    setupClips(env);
    startClips(env);

    /*
     * 测试Logger
     */
    test_logger(env);

    /*
     * 测试Debug
     */
    test_debug(env);

    /*
     * 测试add_function (目前最多支持4个参数)
     */
    test_add_function(env);

    /*
     * 测试Module
     */
    test_module(env);

    /*
     * 测试Global
     */
    test_global(env);

    /*
     * 测试Function
     */
    test_function(env);

    /*
     * 测试Template
     */
    test_template(env);

    /*
     * 测试Rule
     */
    test_rule(env);

    return 0;
}/*}}}*/
