OBJ = main.obj

conge3d.exe: $(OBJ)
	$(CC) /Fe:$@ $**

.PHONY: clean

clean:
	-rm conge3d.exe $(OBJ)
