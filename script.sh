#!/bin/bash


if [[ $1 == 1 ]]; then
	OUTDIR="results/"
fi

for file in instances/*
do
	if [[ $1 == 1 ]]; then
		for i in {1..10};
		do
			(time ./SA $file) 2>> $OUTDIR${file:10}
		done
	fi

done

echo "Finish $FILE"
