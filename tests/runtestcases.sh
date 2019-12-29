passes=0
fails=0


# testbvo {{{
testbvo() {
	test1() {
		# Setup
		touch tmp
		../G2ME -b input/TEST1 > /dev/null

		# Test 1
		../G2ME -o tmp
		if cmp tmp correct_output/testb1o ; then
			passes=$((passes+1))
			echo -n "."
		else
			fails=$((fails+1))
			echo -n "E"
		fi
		# Test 2
		../G2ME -vo tmp
		if cmp tmp correct_output/testb1vo ; then
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
		../G2ME -o tmp
		if cmp tmp correct_output/testb2o ; then
			passes=$((passes+1))
			echo -n "."
		else
			fails=$((fails+1))
			echo -n "E"
		fi
		# Test 2
		../G2ME -vo tmp
		if cmp tmp correct_output/testb2vo ; then
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


# testBvo {{{
testBvo() {
	# Setup
	touch tmp
	../G2ME -B input/TEST.sea > /dev/null

	# Test 1
	../G2ME -o tmp
	if cmp tmp correct_output/testBo ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 2
	../G2ME -vo tmp
	if cmp tmp correct_output/testBvo ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
}
# }}}


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
	# Test 3
	../G2ME -f input/filter1 -M > tmp
	if cmp tmp correct_output/testM3 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 4
	../G2ME -f input/filter1 -m 2 -M > tmp
	if cmp tmp correct_output/testM4 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
}
# }}}


# testC {{{
testC() {
	# Setup
	touch tmp
	../G2ME -B input/TEST.sea > /dev/null

	# Test 1
	../G2ME -C > tmp
	if cmp tmp correct_output/testC1 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 2
	../G2ME -m 2 -C > tmp
	if cmp tmp correct_output/testC2 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 3
	../G2ME -f input/filter1 -C > tmp
	if cmp tmp correct_output/testC3 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 4
	../G2ME -f input/filter1 -m 2 -C > tmp
	if cmp tmp correct_output/testC4 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
}
# }}}


# testh {{{
testh() {
	# Setup
	touch tmp
	../G2ME -B input/TEST.sea > /dev/null

	# Test 1
	../G2ME -h Dylan > tmp
	if cmp tmp correct_output/testh1 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 2
	../G2ME -h Michael > tmp
	if cmp tmp correct_output/testh2 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 3
	../G2ME -vh Victoria > tmp
	if cmp tmp correct_output/testh3 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
	# Test 4
	../G2ME -vh Aiden > tmp
	if cmp tmp correct_output/testh4 ; then
		passes=$((passes+1))
		echo -n "."
	else
		fails=$((fails+1))
		echo -n "E"
	fi
}
# }}}


runalltests() {
	testbvo
	testBvo
	testbvO
	testBvO
	testM
	testC
	testh
}


runalltests
total=$((fails+passes))

if [ "$fails" -eq 0 ]; then
	echo "${passes}/${total} All tests passed!"
else
	echo "${passes}/${total} passed. $fails tests failed."
fi
