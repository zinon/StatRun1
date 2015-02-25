#!/bin/bash

cd NLLscanner/

root -b -l -x <<EOF

TString s = gSystem->pwd();

string path = s + "/";

cout<<"Compile pwd : "<<path<<endl;

gSystem->CompileMacro( "MakeNLLProfile.C","gk" );

gROOT->ProcessLine(".q");

cout<<"Compile done!"<<endl;

.q
EOF

cd -