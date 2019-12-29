passes=0
fails=0

# testbvO {{{
testbvO() {
	test1() {
		# Setup
		touch tmp
		../G2ME -b input/TEST1 > /dev/null

		# Test 1
		../G2ME -O > tmp
		if cmp tmp correct_output/testb1O ; then
			passes=$((passes+1))
			echo -n "."
		else
			fails=$((fails+1))
			echo -n "E"
		fi
		# Test 2
		../G2ME -vO > tmp
		if cmp tmp correct_output/testb1vO ; then
			passes=$((passes+1))
			echo -n "."
		else
			fails=$((fails+1))
			echo -n "E"
		fi
	}

	test2() {
		# Setup
		touch tmp
		../G2ME -b input/TEST2 > /dev/null

		# Test 1
		../G2ME -O > tmp
		if cmp tmp correct_output/testb2O ; then
			passes=$((passes+1))
			echo -n "."
		else
			fails=$((fails+1))
			echo -n "E"
		fi
		# Test 2
		../G2ME -vO > tmp
		if cmp tmp correct_output/testb2vO ; then
			passes=$((passes+1))
			echo -n "."
		else
			fails=$((fails+1))
			echo -n "E"
		fi
	}

	# Run all the tests
	test1
	test2
}
# }}}


# testBvO {{{
testBvO() {
	# Setup
	touch tmp
	../G2ME -B input/TEST.sea > /dev/null

	# Test 1
	../G2ME -O > tmp
	if cmp tmp correct_output/testBO ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 2
	../G2ME -vO > tmp
	if cmp tmp correct_output/testBvO ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
}
# }}}


# testM {{{
testM() {
	# Setup
	touch tmp
	../G2ME -B input/TEST.sea > /dev/null

	# Test 1
	../G2ME -M > tmp
	if cmp tmp correct_output/testM1 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 2
	../G2ME -m 2 -M > tmp
	if cmp tmp correct_output/testM2 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
}
# }}}


runalltests() {
	testbvO
	testBvO
	testM
}


runalltests
total=$((fails+passes))

if [ "$fails" -eq 0 ]; then
	echo "${passes}/${total} All tests passed!"
else
	echo "${passes}/${total} passed. $fails tests failed."
fi
