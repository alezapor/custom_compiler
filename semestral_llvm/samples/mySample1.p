program mySample1;

function power_of(a : integer; base : integer) : integer;
	begin
		power_of := 1;
		while (a > 0) do
			begin
				power_of := base * power_of;
				dec(a);
			end;
	end;		

var
	exponent, i : integer;
	arr : array [-10 .. 0] of integer;
begin
	for i:=-10 to 0 do
		arr[i] := i+11;
	
    readln(exponent);

    for i:=-10 to 0 do
		begin
    		arr[i] := power_of(exponent, arr[i]);
			writeln(arr[i]);
		end;
end.

