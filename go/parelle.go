package main;

import "fmt";
import "runtime";

var quit chan int = make(chan int);

func loop(id int) {
    for i := 0; i < 10; i++ {
        fmt.Printf("%d ", id);
    }
    quit <- 0;
}

func main() {
    runtime.GOMAXPROCS(2);

    var loopN int = 5;

    for i := 0; i < loopN; i++ {
        go loop(i);
    }

    for i := 0; i < loopN; i++ {
        <- quit;
    }
}
