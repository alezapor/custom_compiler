program factorialCycle;

var
    n: integer;
    f: integer;
begin
    f := 1;
    readln(n);
    while n > 1 do begin
        f := f * n;
		n := n - 1;
    end;
    writeln(f);
end.
