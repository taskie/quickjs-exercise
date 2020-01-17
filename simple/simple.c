#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <quickjs.h>

int main(void) {
  JSRuntime *rt = JS_NewRuntime();
  JSContext *ctx = JS_NewContext(rt);

  char *const fooCode = "function foo(x, y) { return x + y; }";
  if (JS_IsException(JS_Eval(ctx, fooCode, strlen(fooCode), "<input>", JS_EVAL_FLAG_STRICT))) {
    JS_FreeContext(ctx);
    JS_FreeRuntime(rt);
    return -1;
  }

  JSValue global = JS_GetGlobalObject(ctx);
  JSValue foo = JS_GetPropertyStr(ctx, global, "foo");
  JSValue argv[] = { JS_NewInt32(ctx, 5), JS_NewInt32(ctx, 3) };
  JSValue jsResult = JS_Call(ctx, foo, global, sizeof(argv) / sizeof(JSValue), argv);
  int32_t result;
  JS_ToInt32(ctx, &result, jsResult);
  printf("Result: %d\n", result);

  JSValue used[] = { jsResult, argv[1], argv[0], foo, global };
  for (int i = 0; i < sizeof(used) / sizeof(JSValue); ++i) {
    JS_FreeValue(ctx, used[i]);
  }

  JS_FreeContext(ctx);
  JS_FreeRuntime(rt);
  return 0;
}
