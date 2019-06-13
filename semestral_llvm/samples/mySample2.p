program mySample2;

const I1 = 547;
	  I2 = 449;

var ARR : array [-1 .. 21] of integer; 
	I, J, min_id, temp: integer;

begin 
	for I := 21 downto -1 do
		ARR[I] := (22-I) * I2 mod I1;
	for I := 0 to 21 do
		writeln(ARR[I]);
	
	for I:=-1 to 20 do
		begin
			min_id := ;
			
			for J := I+1 to 21 do
				begin
					if (ARR[J] < ARR[min_id]) then
						min_id := J;
				end;
			temp := ARR[I];
			ARR[I] := ARR[min_id];
			ARR[min_id] := temp;
		end;
	for I := -1 to 21 do
		writeln(ARR[I]);
end.
	
