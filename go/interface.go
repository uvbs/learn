package main;

import "fmt";
import "math";
import "reflect";

type Shape interface {
    area() float64;
};


type Circle struct {
    x, y, radius float64;
};

func(circle Circle) area() float64 {
    return math.Pi * circle.radius * circle.radius;
}

func main() {
    fmt.Println("test\n");

    circle := Circle{x:0, y:0, radius:5};
    fmt.Printf("Circle area:%f\n", circle.area());

    t := reflect.TypeOf(circle);
    v := reflect.ValueOf(circle);

    fmt.Print(t);
    fmt.Print("\n");
    fmt.Print(v);
    fmt.Print("\n");

    f := v.MethodByName("area");
    fmt.Print(f);
    fmt.Print("\n");

    args := make([]reflect.Value, 0);

    ret := f.Call(args);
    fmt.Printf("\narea:%d\n", ret[0]);
}
