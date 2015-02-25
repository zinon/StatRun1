#!/usr/bin/python

import os, glob, subprocess, sys

def get1(title):
  s=title.replace('_', ' ')
  return (
  "\documentclass[10pt]{report}\n"
  "\usepackage[utf8]{inputenc}\n"
  "\usepackage{color}\n"
  "\usepackage[paperwidth=700px, paperheight=700px, top=50px,bottom=50px,left=50px,right=50px,]{geometry}\n"
  "\usepackage{hyperref}\n"
  "\usepackage[all]{hypcap}\n" 
  "\hypersetup{colorlinks, breaklinks, urlcolor=red, linkcolor=magenta, citecolor=blue}\n"
  "\usepackage{graphicx}\n"
  "\DeclareGraphicsExtensions{.eps}\n"
  "% \graphicspath{ {./} }\n"
  "\usepackage{epsfig,bm,epsf,float}\n"
  "% Title Page\n"
  "\\title{ { \Large H TauTau HadHad 7+8 TeV} \\newline "+s+"}\n"
  "% document\n"
  "\\begin{document}\n"
  "\maketitle\n"
  "\clearpage \n"
  "\\addcontentsline{toc}{chapter}{\listfigurename}\n"
  "\listoffigures\n"
  "\label{pg:first}\n"
  )
  
 
#============================================================================
def get2():
  return "\\end{document}\n" 
#============================================================================ 
def entry(s0, s1):
  return ("\\begin{figure}[htb]\n"
  "\centering\n"
  "\includegraphics {"+s0+"}\n"
  "\caption["+s1+" ]{"+s1+" - \hyperref[pg:first]{Go to Index} }\n" 
  "\label{fig:"+s0+"}\n"
  "\\end{figure}\n"
  "\clearpage\n"  )  

#============================================================================ 
ref= ""; goal="NP NLL scans"; plots="NLLscans"
#ref = "mu"; goal="NP Vs "+ref; plots="NLLdep"

WSname="Run1PaperHadhad_MVA_r160507"
tag="Apr28"



foldername=plots+"."+WSname+"."+tag
project = goal+" "+WSname+" Combination 2011+2012"
dire 	= "./eps/"+foldername 


if ref:
	wc = '*'+ref+'*.eps'
else:
	wc = "*.eps"
images 	= glob.glob( os.path.join(dire, wc) )

if not images:
  print "no images selected"
  sys.exit()

f = open('images_latex.tex','w')

f.write(get1(project))

for i in images:
  s = i[i.rfind('/')+1:].replace('.eps', '').replace('_', ' ')
  f.write( entry(i,s) )
  
f.write( get2() )
  
  
f.close()


subprocess.call("./dolatex.sh "+foldername, shell=True)
  
