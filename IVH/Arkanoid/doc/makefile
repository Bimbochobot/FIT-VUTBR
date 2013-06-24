NAME=proj
TMP=$(NAME)tmp

$(NAME).pdf: $(TMP).ps
	ps2pdf $(TMP).ps $(NAME).pdf	
    
$(TMP).ps: $(TMP).dvi
	dvips -t a4 $(TMP).dvi
    
$(TMP).dvi: $(TMP).tex title.tex
	latex $(TMP).tex
	latex $(TMP).tex

$(TMP).tex: $(NAME).tex
	vlna -f $(NAME).tex $(TMP).tex

clean:
	-rm $(TMP).dvi $(TMP).ps $(TMP).aux $(TMP).log $(TMP).out $(TMP).tex
