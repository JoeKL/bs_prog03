# Pfade zu den Semaphoren
SEMA = /dev/shm/sem.north /dev/shm/sem.east /dev/shm/sem.south /dev/shm/sem.west

# Pfade zu den FIFOs
FIFOS = police

# Compiler und Flags
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -D_GNU_SOURCE

# default target: Aufgabe C starten
.PHONY: default
default: runC

# Aufgabe 1 starten
.PHONY: runA
runA: Cross_a $(SEMA)
	./Cross_a north west &
	./Cross_a east north &
	./Cross_a south east &
	./Cross_a west south &

# Aufgabe 2 starten
.PHONY: runB
runB: Cross_b Police_b $(SEMA) $(FIFOS)
	./Cross_b north west &
	./Cross_b east north &
	./Cross_b south east &
	./Cross_b west south &
	./Police_b $$(pgrep Cross_b) &

# Aufgabe 3 starten
.PHONY: runC
runC: Cross_c Police_c $(SEMA) $(FIFOS)
	./Cross_c north west &
	./Cross_c east north &
	./Cross_c south east &
	./Cross_c west south &
	./Police_c $$(pgrep Cross_c) &

# Käfer und Polizei hart beenden und Semaphoren zurücksetzen
.PHONY: kill
kill: setup
	-killall Cross_a Cross_b Cross_c Police_b Police_c
	./setup

# INT Signal an Käfer und Polizei senden
.PHONY: stop
stop:
	-killall -INT Cross_a Cross_b Cross_c Police_b Police_c

# versch. Aufgabenteile bauen

Cross_a: Cross_a.c
	$(CC) $(CFLAGS) Cross_a.c -o Cross_a

Cross_b: Cross_b.c
	$(CC) $(CFLAGS) Cross_b.c -o Cross_b

Cross_c: Cross_c.c
	$(CC) $(CFLAGS) Cross_c.c -o Cross_c

Police_b: Police_b.c
	$(CC) $(CFLAGS) Police_b.c -o Police_b

Police_c: Police_c.c
	$(CC) $(CFLAGS) Police_c.c -o Police_c

setup: setup.c
	$(CC) $(CFLAGS) setup.c -o setup

# Semaphoren und FIFOs erstellen

$(SEMA): setup
	./setup

$(FIFOS):
	mkfifo $(FIFOS)

# Semaphoren, FIFOs und Programme löschen
.PHONY: clean
clean: setup
	./setup
	rm -rf setup Police_b Police_c Cross_a Cross_b Cross_c $(FIFOS)
