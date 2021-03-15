OBJ = main.obj vmath.obj gmath.obj

conge3d.exe: $(OBJ)
	$(CC) /Fe:$@ $**

main.obj: vmath.h gmath.h conge/conge.h
vmath.obj: vmath.h
gmath.obj: vmath.h gmath.h

.PHONY: clean

clean:
	-rm conge3d.exe $(OBJ)
