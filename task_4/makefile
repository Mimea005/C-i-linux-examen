CC = gcc
CFLAGS = -O0 -ggdb -I$(GLOBALINCLUDE)
GLOBALINCLUDE = include

BINDIR = bin
LIBDIR = lib
OBJDIR = obj
OUTDIR = .

.PHONY: all clean run mem
.DEFAULT: all

#	Template
libfile = $(LIBDIR)/$(1)/$(1).a
requiredlibs = $(patsubst $(BINDIR)/$(1)/include/%.h,$(OUTDIR)/%.a,$(wildcard $(BINDIR)/$(1)/include/*.h))

####################
#	Generate lists
####################
ALLTARGETS = $(patsubst $(LIBDIR)/%/,$(OUTDIR)/%.a, $(wildcard $(LIBDIR)/*/)) $(patsubst $(BINDIR)/%/, $(OUTDIR)/%, $(wildcard $(BINDIR)/*/))

####################
#	Compile or move libraries
####################
$(OBJDIR)/%.o: $(LIBDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $? -o $@ $(file < $(LIBDIR)/$(dir $*)/cflags)

.SECONDEXPANSION:
$(OUTDIR)/%.a: $$(call libfile,%)
	@mkdir -p $(dir $@)
	cp $< $@

$(OUTDIR)/%.a: $$(foreach src, $$(wildcard $(LIBDIR)/%/*.c), $$(patsubst $(LIBDIR)/%.c, $(OBJDIR)/%.o, $$(src)))
	@mkdir -p $(dir $@)
	$(AR) rcs $@ $+

####################
#	Compile binaries
####################
$(OUTDIR)/%: $(BINDIR)/%/*.c $$(call requiredlibs,%)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(BINDIR)/$*/include -o $@ $+ $(file < $(BINDIR)/$*/cflags)

all: $(ALLTARGETS)

clean:
	rm $(ALLTARGETS)
	rm -rf $(OBJDIR)/*.o
	find ./ -depth -empty -type d -delete
