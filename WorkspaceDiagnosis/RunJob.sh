#!/bin/sh
show_structure(){
find . -print | awk 'NR>1{printf "%s",$0;if (system("[ -d \"" $0 "\" ]") == 0) printf "/";printf "\n"}'| LC_ALL=C sort|awk 'BEGIN{system("pwd")}{n=split($0,a,"/");for (i=2;i<n;i++) if (i<n-1 || a[n]!="") {printf "| "}; if (a[n]=="") {print "|___[D]"a[n-1]} else {print "|___[F]"a[n]}}'| tac| awk '{n=split($0,a,"");for(i=1;i<=n;i++) {if (a[i]=="|" && b[i]!="|" && a[i+1]!="_") a[i]=" "; b[i]=a[i];printf "%s",b[i]}; print ""}'| tac
}

workplace=${PWD}

echo "Working at " $workplace

ls -lh

echo "Using root ..."

which root

# transforming the root file
echo "recovering the root file ..."

expFile=<EXPECTEDFILE>
Folder=<FOLDER>

if [ ! -d "$Folder" ]; then
  echo "$Folder does not exist"; exit 1;
fi

cd $Folder

ls -lh

if [ ! -f $expFile.ntup ]; then
  echo "Expected file $expFile not found in $Folder"; exit 1;
else
  echo "Expected file $expFile succesfully found in $Folder"
  mv $expFile.ntup $expFile.root
  ls -lh
fi

cd $workplace

# tree $workplace
#show_structure

cd FitFunction/; root Compile.C; cd ../

cd NLLProfileScans; source compile.sh;

#run
root -b -l -x root TmpRunCode.C

echo "root content in current folder ${PWD}"
ls -lh *.root

mv *.root ../

cd ../

echo "root content in current folder ${PWD}"
ls -lh *.root
stat *.root

show_structure

echo "Done!"
