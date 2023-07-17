#!/bin/ksh

PROG="../search"
EXIT_CODE=0

srch()
{
	typeset max_err="$1"; shift
	typeset pattern="$1"; shift
	typeset file="$1"; shift
	typeset lines="$1"; shift

 	echo "<<< k=$max_err pat=$pattern $file"
	count=$($PROG -b -k $max_err "$pattern" $file | tee search.tmp | wc -l)
	cat search.tmp

	printf '     expect=%d got=%d' $lines $count
	if [ $count -ne $lines ]; then
		printf '\rFAIL\n'
		EXIT_CODE=1
		return 1
	fi
	printf '\r-OK-\n'

	printf '     check offsets '
	for offset in $( cut -d' ' -f2 search.tmp ) ; do
		typeset expect="$1"; shift
		if [ $offset -ne $expect ]; then
			printf '\rFAIL\n'
			EXIT_CODE=1
			return 1
		fi
		printf "$offset "
	done
	printf '\r-OK-\n'

	return 0
}

srch 0 AGCT AGCT.txt 2 12 0
srch 1 AGCT AGCT.txt 2 12 0
srch 2 AGCT AGCT.txt 2 2 0
srch 3 AGCT AGCT.txt 2 2 0
#srch 4 AGCT AGCT.txt 2 0 0

srch 0 GCAGAGAG GCAGAGAG.txt 2 5 0
srch 1 GCAGAGAG GCAGAGAG.txt 3 5 10 0
srch 2 GCAGAGAG GCAGAGAG.txt 3 5 10 0
srch 3 GCAGAGAG GCAGAGAG.txt 3 3 10 0
srch 4 GCAGAGAG GCAGAGAG.txt 3 0 1 0

srch 0 GATTACA GATTACA.txt 3 12 12 0
srch 1 GATTACA GATTACA.txt 4 12 12 8 0

srch 0 return return.txt 4 12 0 8 13
srch 1 return return.txt 5 12 0 8 13 9
srch 2 return return.txt 6 12 0 8 13 9 9
srch 3 return return.txt 8 12 0 8 13 9 9 9 10

rm search.tmp

exit ${EXIT_CODE}
