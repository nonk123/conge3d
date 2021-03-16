OBJ = main.obj vmath.obj gmath.obj mesh.obj

conge3d.exe: $(OBJ)
	cd conge
	$(MAKE)
	cd ..
	$(CC) /Fe:$@ $** /link conge\conge.lib user32.lib

main.obj: vmath.h gmath.h mesh.h conge/conge.h
vmath.obj: vmath.h
gmath.obj: gmath.h vmath.h conge/conge.h
mesh.obj: mesh.h vmath.h gmath.h conge/conge.h

.PHONY: clean

clean:
	-rm conge3d.exe $(OBJ)
