PrgName = xwinclone

CC      = gcc

CFLAGS  = -Wall -pedantic -std=c11

ODIR    = obj

LIBS    = -lX11 -lXmu -lXcomposite

DEPS    = defines.h headers.h xwclib.h

_OBJ    = xwclib.o xwinclone.o


###############################################################################

OBJ     = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
		@echo compiling $@
		$(CC) -c -o $@ $< $(CFLAGS)

$(PrgName): $(OBJ)
		@echo linking $@
		gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
		rm -f $(ODIR)/*.o $(PrgName)
