#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <streambuf>
#include <string>
#include <type_traits>

#include <cstdio>
#include <cstring>

#include <quickjs.h>

JSValue jsFprint(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv, FILE *f) {
  for (int i = 0; i < argc; ++i) {
    if (i != 0) {
      std::fputc(' ', f);
    }
    const char *str = JS_ToCString(ctx, argv[i]);
    if (!str) {
      return JS_EXCEPTION;
    }
    std::fputs(str, f);
    JS_FreeCString(ctx, str);
  }
  std::fputc('\n', f);
  std::fflush(f);
  return JS_UNDEFINED;
}

JSValue jsPrint(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
  return jsFprint(ctx, jsThis, argc, argv, stdout);
}

JSValue jsPrintErr(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
  return jsFprint(ctx, jsThis, argc, argv, stderr);
}

// Mt19937 class の一意なID（後段で初期化）
// 簡単のためグローバルに定義するが、複数 Runtime を同時に動かした時に破綻するので適宜やっていく必要がある
static JSClassID jsMt19937ClassID;

// Mt19937.prototype.generate
JSValue jsMt19937Generate(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
  std::mt19937 *p = static_cast<std::mt19937 *>(JS_GetOpaque(jsThis, jsMt19937ClassID));
  return JS_NewBigUint64(ctx, (*p)());
}

// Mt19937.prototype
const JSCFunctionListEntry jsMt19937ProtoFuncs[] = {
  JS_CFUNC_DEF("generate", 1, jsMt19937Generate),
};

// Mt19937 の constructor
JSValue jsMt19937New(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
  // インスタンスを生成
  JSValue obj = JS_NewObjectClass(ctx, jsMt19937ClassID);
  bool fail = false;
  if (argc == 0) {
    JS_SetOpaque(obj, new std::mt19937());
  } else if (argc == 1) {
    if (JS_IsInteger(argv[0])) {
      uint32_t val;
      int err = JS_ToUint32(ctx, &val, argv[0]);
      if (err) {
        fail = true;
      } else {
        JS_SetOpaque(obj, new std::mt19937(val));
      }
    } else {
      fail = true;
    }
  } else {
    fail = true;
  }
  if (fail) {
    JS_FreeValue(ctx, obj);  // 忘れがち
    return JS_EXCEPTION;
  }
  return obj;
}

// Mt19937 object が GC に回収された際に呼ばれる
void jsMt19937Finalizer(JSRuntime *rt, JSValue val) {
  std::mt19937 *p = static_cast<std::mt19937 *>(JS_GetOpaque(val, jsMt19937ClassID));
  delete p;
}

// Mt19937 class の定義
// JS 側に表出していないオブジェクト間の依存がある場合 .gc_mark もアレコレする必要があるっぽい
JSClassDef jsMt19937Class = {
  "Mt19937",
  .finalizer = jsMt19937Finalizer,
};

// rand module 内の関数一覧
static const JSCFunctionListEntry randFuncs[] = {
  JS_CFUNC_SPECIAL_DEF("Mt19937", 1, constructor, jsMt19937New),  // new Mt19937() できるようにする
  // JS_CFUNC_DEF("Mt19937", 1, jsMt19937New),  // Mt19937() としたいならこっち
};

// rand module の初期化（JS の import 時に呼ばれるやつ）
int initRand(JSContext *ctx, JSModuleDef *m) {
  JS_NewClassID(&jsMt19937ClassID);
  JS_NewClass(JS_GetRuntime(ctx), jsMt19937ClassID, &jsMt19937Class);
  // prototype 設定
  JSValue mt19937Proto = JS_NewObject(ctx);
  JS_SetPropertyFunctionList(ctx, mt19937Proto, jsMt19937ProtoFuncs, std::extent_v<decltype(jsMt19937ProtoFuncs)>);
  JS_SetClassProto(ctx, jsMt19937ClassID, mt19937Proto);
  // 最後の引数は sizeof(randFuncs) / sizeof(JSCFunctionListEntry) の意
  return JS_SetModuleExportList(ctx, m, randFuncs, std::extent_v<decltype(randFuncs)>);
}

// rand module の定義
JSModuleDef *initRandModule(JSContext *ctx, const char *moduleName) {
  JSModuleDef *m = JS_NewCModule(ctx, moduleName, initRand);
  if (!m) {
    return nullptr;
  }
  JS_AddModuleExportList(ctx, m, randFuncs, std::extent_v<decltype(randFuncs)>);
  return m;
}

void initContext(JSContext *ctx) {
  JSValue global = JS_GetGlobalObject(ctx);

  // globalThis に console を追加
  JSValue console = JS_NewObject(ctx);
  JS_SetPropertyStr(ctx, global, "console", console);
  // console.log を設定
  JS_SetPropertyStr(ctx, console, "log", JS_NewCFunction(ctx, jsPrint, "log", 1));
  // console.error を設定
  JS_SetPropertyStr(ctx, console, "error", JS_NewCFunction(ctx, jsPrintErr, "error", 1));

  // rand module の登録
  initRandModule(ctx, "rand");

  JS_FreeValue(ctx, global);
}

std::string readFile(std::string &filename) {
  std::ifstream t {filename};
  return std::string {std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>()};
}

int main(int argc, char const *argv[]) {
  std::unique_ptr<JSRuntime, decltype(&JS_FreeRuntime)> uRt {JS_NewRuntime(), JS_FreeRuntime};
  std::unique_ptr<JSContext, decltype(&JS_FreeContext)> uCtx {JS_NewContext(uRt.get()), JS_FreeContext};
  JSContext *ctx = uCtx.get();

  initContext(ctx);

  for (int i = 1; i < argc; ++i) {
    std::string filename = argv[i];
    std::string code = readFile(filename);
    // JS_EVAL_TYPE_MODULE でないと import できない
    JSValue ret = JS_Eval(ctx, code.c_str(), code.size(), "<input>", JS_EVAL_TYPE_MODULE | JS_EVAL_FLAG_STRICT);
    if (JS_IsException(ret)) {
      JSValue e = JS_GetException(ctx);
      jsPrintErr(ctx, JS_NULL, 1, &e);
      JS_FreeValue(ctx, e);
      return 1;
    } else if (JS_IsUndefined(ret)) {
      // nop
    } else {
      jsPrint(ctx, JS_NULL, 1, &ret);
    }
    JS_FreeValue(ctx, ret);
  }

  return 0;
}
