program expressions2;

var
    x, y, a, b, c, d: integer;

begin
    readln(x);
    readln(y);

    a := x + y;
    b := y - x;

    writeln(x);
    writeln(y);
    writeln(a);
    writeln(b);

    c := (x + a) * (y - b);

    writeln(c);

    d := a mod b;

    writeln(d);
end.
