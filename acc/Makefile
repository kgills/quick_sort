CC=pgcc
C_FLAGS=-O4 -mcmodel=medium -acc -Munroll -ta=tesla,8.0 -Minfo=accel
# C_FLAGS=-O4 -mcmodel=medium -Munroll
OUTPUT=quick.out

all: quick.c
	$(CC) $(C_FLAGS) $^ -o $(OUTPUT)

clean:
	@rm -f $(OUTPUT)
	@rm -f slurm*
