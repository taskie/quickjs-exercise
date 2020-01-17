#include <stdio.h>
#include <string.h>

#include <quickjs.h>

JSValue jsFprint(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv, FILE *f) {
  for (int i = 0; i < argc; ++i) {
    if (i != 0) {
      fputc(' ', f);
    }
    const char *str = JS_ToCString(ctx, argv[i]);
    if (!str) {
      return JS_EXCEPTION;
    }
    fputs(str, f);
    JS_FreeCString(ctx, str);
  }
  fputc('\n', f);
  return JS_UNDEFINED;
}

JSValue jsPrint(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
  return jsFprint(ctx, jsThis, argc, argv, stdout);
}

JSValue jsPrintErr(JSContext *ctx, JSValueConst jsThis, int argc, JSValueConst *argv) {
  return jsFprint(ctx, jsThis, argc, argv, stderr);
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

  JS_FreeValue(ctx, global);
}

int main(int argc, char const *argv[]) {
  int exitCode = 0;

  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  initContext(ctx);

  for (int i = 1; i < argc; ++i) {
    JSValue ret = JS_Eval(ctx, argv[i], strlen(argv[i]), "<input>", JS_EVAL_FLAG_STRICT);
    if (JS_IsException(ret)) {
      JSValue e = JS_GetException(ctx);
      jsPrintErr(ctx, JS_NULL, 1, &e);
      JS_FreeValue(ctx, e);
      exitCode = 1;
      break;
    } else if (JS_IsUndefined(ret)) {
      // nop
    } else {
      jsPrint(ctx, JS_NULL, 1, &ret);
    }
    JS_FreeValue(ctx, ret);
  }

  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return exitCode;
}
