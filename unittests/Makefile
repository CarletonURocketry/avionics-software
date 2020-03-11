MAKEFILES = $(wildcard ./*/Makefile)
TEST_DIRS = $(dir $(MAKEFILES))
COVERAGE_FILES = $(addsuffix total_coverage.info,$(TEST_DIRS))
TEST_CLEANS = $(addsuffix clean,$(TEST_DIRS))

report: lcov_report
	lcov --rc lcov_branch_coverage=1 --summary coverage.info

lcov_report: coverage.info
	# Generate HTML report
	genhtml --branch-coverage $< --output-directory $@

coverage.info: $(COVERAGE_FILES)
	# Combine all of the coverage reports
	ls ./*/total_coverage.info | sed 's/^/-a /' | xargs lcov --rc lcov_branch_coverage=1 -o $@

$(COVERAGE_FILES):
	make -C $(dir $@) -j $(notdir $@)

clean:
	rm -f coverage.info
	rm -rf lcov_report

$(TEST_CLEANS):
	make -C $(dir $@) -j clean

clean_all: clean $(TEST_CLEANS)

.PHONY : build report clean clean_all $(TEST_CLEANS)

