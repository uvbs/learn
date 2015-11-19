package main;

import fmt "fmt"

func max(a int, b int) int {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

func min(a int, b int) int {
    return a;
}

func main() {
    fmt.Printf("hello, world!\n");

    var a, b int;
    a = 10;
    b = 20;

    fmt.Printf("max:%d\n", max(a, b));
}
