package main;

var c = make(chan int, 10);
var a string;

func hello() {
    a = "this\n";
    go func() {
        a = "Hello";
    }();
    print(a);
}

func f() {
    a = "This in f\n";
    c <- 0;
}

func main() {
    go f();
    <-c;
    print(a);
}
