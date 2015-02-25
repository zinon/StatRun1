NOW=$(date +"%k%M%d%m%Y")
tag=$1
latex images_latex.tex; latex images_latex.tex; dvips images_latex.dvi; ps2pdf images_latex.ps; mv images_latex.pdf ${tag}_latex_plots_${NOW}.pdf;  ls #evince images_latex_${NOW}.pdf &
