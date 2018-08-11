MDS    = $(shell find -s src -name "*.md")
OUTPUT = ambook.html

RENDER_LIST = $(foreach f, $(MDS), $(f) .blankline)

all:
	@mkdir -p build/
	pandoc -f markdown+footnotes \
	       -t html src/title.txt $(RENDER_LIST) > $(OUTPUT)
