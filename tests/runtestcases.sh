passes=0
fails=0

PRODUCED_OUTPUT="produced_output"
EXPECTED_OUTPUT="correct_output"


testB1A() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1A1"
		"testB1A2"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -A Jayden"
		"../G2ME -A Harper"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testb1vo() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST1 > /dev/null

	TEST_NAME_LIST=(
		"testb1o"
		"testb1vo"
		"testb1mo"
		"testb1mvo"
		"testb1fo"
		"testb1fvo"
		"testb1mfo"
		"testb1mfvo"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -o"
		"../G2ME -vo"
		"../G2ME -m 2 -o"
		"../G2ME -m 2 -vo"
		"../G2ME -f input/filter1 -o"
		"../G2ME -f input/filter1 -vo"
		"../G2ME -m 2 -f input/filter1 -o"
		"../G2ME -m 2 -f input/filter1 -vo"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, appending the filepath for the output file in
		# the produced output directory
		$(${testcommand} ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testb2vo() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST2 > /dev/null

	TEST_NAME_LIST=(
		"testb2o"
		"testb2vo"
		"testb2mo"
		"testb2mvo"
		"testb2fo"
		"testb2fvo"
		"testb2mfo"
		"testb2mfvo"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -o"
		"../G2ME -vo"
		"../G2ME -m 2 -o"
		"../G2ME -m 2 -vo"
		"../G2ME -f input/filter1 -o"
		"../G2ME -f input/filter1 -vo"
		"../G2ME -m 2 -f input/filter1 -o"
		"../G2ME -m 2 -f input/filter1 -vo"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, appending the filepath for the output file in
		# the produced output directory
		$(${testcommand} ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testB1vo() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1o"
		"testB1vo"
		"testB1mo"
		"testB1mvo"
		"testB1fo"
		"testB1fvo"
		"testB1mfo"
		"testB1mfvo"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -o"
		"../G2ME -vo"
		"../G2ME -m 2 -o"
		"../G2ME -m 2 -vo"
		"../G2ME -f input/filter1 -o"
		"../G2ME -f input/filter1 -vo"
		"../G2ME -m 2 -f input/filter1 -o"
		"../G2ME -m 2 -f input/filter1 -vo"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, appending the filepath for the output file in
		# the produced output directory
		$(${testcommand} ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testb1vO() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST1 > /dev/null

	TEST_NAME_LIST=(
		"testb1O"
		"testb1vO"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -O"
		"../G2ME -vO"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testb2vO() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -b input/TEST2 > /dev/null

	TEST_NAME_LIST=(
		"testb2O"
		"testb2vO"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -O"
		"../G2ME -vO"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testB1vO() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1O"
		"testB1vO"
		"testB1mO"
		"testB1mvO"
		"testB1fO"
		"testB1fvO"
		"testB1mfO"
		"testB1mfvO"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -O"
		"../G2ME -vO"
		"../G2ME -m 2 -O"
		"../G2ME -m 2 -vO"
		"../G2ME -f input/filter1 -O"
		"../G2ME -f input/filter1 -vO"
		"../G2ME -m 2 -f input/filter1 -O"
		"../G2ME -m 2 -f input/filter1 -vO"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testB1c() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1c1"
		"testB1c2"
		"testB1c3"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -c Dylan"
		"../G2ME -c Michael"
		"../G2ME -c Valerie"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testB1C() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1C1"
		"testB1C2"
		"testB1C3"
		"testB1C4"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -C"
		"../G2ME -m 2 -C"
		"../G2ME -f input/filter1 -C"
		"../G2ME -f input/filter1 -m 2 -C"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testB1h() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1h1"
		"testB1h2"
		"testB1h3"
		"testB1h4"
		"testB1fvh1"
		"testB1fvh2"
		"testB1mvh1"
		"testB1mvh2"
		"testB1mfvh"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -h Dylan"
		"../G2ME -h Michael"
		"../G2ME -vh Victoria"
		"../G2ME -vh Aiden"
		"../G2ME -f input/filter1 -vh Aiden"
		"../G2ME -f input/filter1 -vh Dylan"
		"../G2ME -m 2 -vh Valerie"
		"../G2ME -m 3 -vh Valerie"
		"../G2ME -m 2 -f input/filter1 -vh Michael"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testB1M() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1M1"
		"testB1M2"
		"testB1M3"
		"testB1M4"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -M"
		"../G2ME -m 2 -M"
		"../G2ME -f input/filter1 -M"
		"../G2ME -f input/filter1 -m 2 -M"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


testB1R() {
	# {{{
	# Setup
	if [[ ! -d "${PRODUCED_OUTPUT}" ]]; then
		mkdir ${PRODUCED_OUTPUT}
	fi
	../G2ME -B input/TEST.sea > /dev/null

	TEST_NAME_LIST=(
		"testB1R1"
		"testB1R2"
		"testB1vR1"
		"testB1vR2"
		"testB1fR1"
		"testB1fR2"
		"testB1fvR1"
		"testB1fvR2"
		"testB1mR1"
		"testB1mR2"
		"testB1mvR1"
		"testB1mvR2"
		"testB1mfvR"
	)
	TEST_COMMAND_LIST=(
		"../G2ME -R Dylan"
		"../G2ME -R Michael"
		"../G2ME -vR Victoria"
		"../G2ME -vR Aiden"
		"../G2ME -f input/filter1 -R Aiden"
		"../G2ME -f input/filter1 -R Dylan"
		"../G2ME -f input/filter1 -vR Victoria"
		"../G2ME -f input/filter1 -vR Michael"
		"../G2ME -m 2 -R Owen"
		"../G2ME -m 1 -R Noah"
		"../G2ME -m 2 -vR Amy"
		"../G2ME -m 3 -vR Owen"
		"../G2ME -m 2 -f input/filter1 -vR Michael"
	)

	# Run all the specified tests 
	# {{{

	# The number of elements in the name list and the command list should be
	# the same
	if [[ "${#TEST_COMMAND_LIST[@]}" -ne "${#TEST_COMMAND_LIST[@]}" ]]; then
		echo "Test suite configured incorrectly."
	fi

	# Print each element of the array 'FILE_LIST' on its own line
	for i in "${!TEST_COMMAND_LIST[@]}"; do
		testname=${TEST_NAME_LIST[$i]}
		testcommand=${TEST_COMMAND_LIST[$i]}

		# Run the test command, putting the output in a dedicated file in
		# the produced output directory
		$(${testcommand} > ${PRODUCED_OUTPUT}/${testname})

		# Compare the output from the test command to the expected output
		if cmp ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} &> /dev/null; then
			passes=$((passes+1))
			echo -n "."
		else
			# If the produced output is not an exact match to the expected
			# output, output the names of the files that differ plus the lines
			# where they differ
			fails=$((fails+1))
			echo "E"
			echo "Files \"${PRODUCED_OUTPUT}/${testname}\" and \"${EXPECTED_OUTPUT}/${testname}\" differ"
			diff -U 0 ${PRODUCED_OUTPUT}/${testname} ${EXPECTED_OUTPUT}/${testname} | tail -n +4
		fi
	done
	# }}}
	# }}}
}


runalltests() {
	testB1A
	testb1vo
	testb2vo
	testB1vo
	testb1vO
	testb2vO
	testB1vO
	testB1c
	testB1C
	testB1h
	testB1M
	testB1R
}


runalltests
total=$((fails+passes))

if [ "$fails" -eq 0 ]; then
	echo "${passes}/${total} All tests passed!"
else
	echo "${passes}/${total} passed. $fails tests failed."
fi
