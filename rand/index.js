import { Mt19937 } from "rand";

const mt = new Mt19937();
for (let i = 0; i < 10; ++i) {
  console.log(mt.generate());  // 乱数 (BigInt) を出力する
}
