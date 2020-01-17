#include <random>
#include <type_traits>

#include <quickjs.h>

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
extern "C" JSModuleDef *js_init_module(JSContext *ctx, const char *moduleName) {
  JSModuleDef *m = JS_NewCModule(ctx, moduleName, initRand);
  if (!m) {
    return nullptr;
  }
  JS_AddModuleExportList(ctx, m, randFuncs, std::extent_v<decltype(randFuncs)>);
  return m;
}
