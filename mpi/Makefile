CC=mpicc
C_FLAGS=-O4 -mcmodel=medium -omp -Munroll
OUTPUT=quick.out

all: quick.c
	$(CC) $(C_FLAGS) $^ -o $(OUTPUT)

clean:
	@rm -f $(OUTPUT)
	@rm -f slurm*
