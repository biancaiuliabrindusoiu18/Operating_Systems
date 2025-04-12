#!/bin/bash


chmod +rw "$1"

pattern_1="corrupted"
pattern_2="dangerous"
pattern_3="risk"
pattern_4="attack"
pattern_5="malware"
pattern_6="malicious"

line_count=$(wc -l<"$1")
word_count=$(wc -w<"$1")
char_count=$(wc -c<"$1")

#echo "nr linii: $line_count"
#echo "nr cuvinte: $word_count"
#echo "nr caractere: $char_count"

ok=1
ok2=1

if test "$line_count" -lt 3 
then
	if test "$word_count" -gt 1000
	then
		if test "$char_count" -gt 2000
		then
			ok2=0
		fi
	fi
fi



if grep -q "$pattern_1" "$1"; then
	ok=0
fi

if grep -q "$pattern_2" "$1"; then
	ok=0
fi

if grep -q "$pattern_3" "$1"; then
	ok=0
fi

if grep -q "$pattern_4" "$1"; then
	ok=0
fi

if grep -q "$pattern_5" "$1"; then
	ok=0
fi

if grep -q "$pattern_6" "$1"; then
	ok=0
fi

if grep -q -P "[^\x00-\x7F]" "$1"; then
	#echo "contine caractere non-ascii"
	ok=0
fi

if test "$ok2" -eq 1      #dc are mai mult de 3 linii <1000 cuv <2000 char blabla
 then
	echo "SAFE"
	chmod -rw "$1"
	exit 0
else
		if test "$ok" -eq 1   #dc nu a gasit cuv aiurea sau caractere on ascii
		 then
			echo "SAFE"
		else
		#mv "$1" "$2"
			echo "$1"
		fi
fi


chmod -rw "$1"
exit 0



