#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct {
    ngx_str_t output_words;
} ngx_http_hello_world_loc_conf_t;

// To process HelloWorld command arguments,define
static char* ngx_http_hello_world(ngx_conf_t* cf, ngx_command_t* cmd, void* conf);

// Allocate memory for HelloWorld command,define
static void* ngx_http_hello_world_create_loc_conf(ngx_conf_t* cf);

// Copy HelloWorld argument to another place,define
static char* ngx_http_hello_world_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child);

// Structure for the HelloWorld command
static ngx_command_t ngx_http_hello_world_commands[] = {
    {
        ngx_string("hello_world"), //首先是指令名称，利用ngx_string宏定义
        NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,//指令属性，可以用“|”将使指令具备多个属性
        ngx_http_hello_world, //定义处理这个指令的回调函数，这个是核心
        NGX_HTTP_LOC_CONF_OFFSET,//指定配置解析后存放在哪里 有三个选择项：NGX_HTTP_MAIN_CONF_OFFSET，NGX_HTTP_SRV_CONF_OFFSET，或者NGX_HTTP_LOC_CONF_OFFSET，表示分别存入main_conf、srv_conf、loc_conf，对应于HTTP全局配置、某个主机的配置和某个URI的配置
        offsetof(ngx_http_hello_world_loc_conf_t, output_words),//承接第四行的配置，表示数据具体保存在main_conf、srv_conf、loc_conf指向的结构体的哪个位置（offset偏移）
        NULL//是一个补充字段，一般不用的，填入NULL
    },
    ngx_null_command//表示结束了
};

// Structure for the HelloWorld context
//各个回调函数的原型。一共是8个，分为4对，没有用到的填为NULL
static ngx_http_module_t ngx_http_hello_world_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_hello_world_create_loc_conf,
    ngx_http_hello_world_merge_loc_conf
};

// Structure for the HelloWorld module, the most important thing
ngx_module_t ngx_http_hello_world_module = {//ngx_http_hello_world_module是我们模块的名字，这个名字编译时也要用到；
    NGX_MODULE_V1,//nginx模块规范的版本号，一致使用NGX_MODULE_V1
    &ngx_http_hello_world_module_ctx,//指向包含解析模块配置文件时用到的回调函数集合的定义的数据结构的指针。这个结构的一般命名法是<模块名>_ctx
    ngx_http_hello_world_commands,//模块提供的指令集的数据结构，这个结构的一般命名法是<模块名>_commands
    NGX_HTTP_MODULE,//模块类别，我们这个模块是HTTP模块，所以值是NGX_HTTP_MODULE
    NULL,//init_master：master进程初始化时调用，直到现在，nginx也没有真正使用过init_master
    NULL,//init_module：master进程解析配置以后初始化模块时调用一次
    NULL,//init_process：worker进程初始化时调用一次
    NULL,//init_thread：多线程时，线程初始化时调用。Unix/Linux环境下未使用多线程
    NULL,//exit_thread：多线程退出时调用
    NULL,//exit_process：worker进程退出时调用一次
    NULL,//exit_master：master进程退出时调用一次
    NGX_MODULE_V1_PADDING//未用字段，使用NGX_MODULE_V1_PADDING补齐
};

static ngx_int_t ngx_http_hello_world_handler(ngx_http_request_t* r) {
    ngx_int_t rc;
    ngx_buf_t* b;
    ngx_chain_t out[2];

    ngx_http_hello_world_loc_conf_t* hlcf;
    hlcf = ngx_http_get_module_loc_conf(r, ngx_http_hello_world_module);

    r->headers_out.content_type.len = sizeof("text/plain") - 1;
    r->headers_out.content_type.data = (u_char*)"text/plain";

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out[0].buf = b;
    out[0].next = &out[1];

    b->pos = (u_char*)"hello_world, ";
    b->last = b->pos + sizeof("hello_world, ") - 1;
    b->memory = 1;

    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

    out[1].buf = b;
    out[1].next = NULL;

    b->pos = hlcf->output_words.data;
    b->last = hlcf->output_words.data + (hlcf->output_words.len);
    b->memory = 1;
    b->last_buf = 1;

    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = hlcf->output_words.len + sizeof("hello_world, ") - 1;
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    return ngx_http_output_filter(r, &out[0]);
}

static void* ngx_http_hello_world_create_loc_conf(ngx_conf_t* cf) {
    ngx_http_hello_world_loc_conf_t* conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_world_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }
    conf->output_words.len = 0;
    conf->output_words.data = NULL;

    return conf;
}

static char* ngx_http_hello_world_merge_loc_conf(ngx_conf_t* cf, void* parent, void* child) {
    ngx_http_hello_world_loc_conf_t* prev = parent;
    ngx_http_hello_world_loc_conf_t* conf = child;
    ngx_conf_merge_str_value(conf->output_words, prev->output_words, "Nginx");
    return NGX_CONF_OK;
}
//Handler入口
static char* ngx_http_hello_world(ngx_conf_t* cf, ngx_command_t* cmd, void* conf) {
    ngx_http_core_loc_conf_t* clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_hello_world_handler;
    ngx_conf_set_str_slot(cf, cmd, conf);
    return NGX_CONF_OK;
}
