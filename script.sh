#!/bin/bash


if [[ $1 == 1 ]]; then
	OUTDIR="resultsThreads/"
fi

if [[ $1 == 2 ]]; then
	OUTDIR="resultsSecuential/"
fi

for file in instances/*
do
	if [[ $1 == 1 ]]; then
		for i in {1..10};
		do
			(time ./SA $file) 2>> $OUTDIR${file:10}
		done
	fi

	if [[ $1 == 2 ]]; then
		for i in {1..10};
		do
			(time ./v2sa $file) 2>> $OUTDIR${file:10}
		done
		echo "Done: ${file:10}"
	fi
done

echo "Finish $FILE"
