
#include "sdb_diff.h"
#include "minunit.h"

static Sdb *test_sdb_new() {
	Sdb *r = sdb_new0 ();
	sdb_set (r, "some", "stuff", 0);
	sdb_set (r, "and", "even", 0);
	sdb_set (r, "more", "stuff", 0);

	sdb_ns (r, "emptyns", true);

	Sdb *test_ns = sdb_ns (r, "test", true);
	sdb_set (test_ns, "a", "123", 0);
	sdb_set (test_ns, "b", "test", 0);
	sdb_set (test_ns, "c", "hello", 0);

	Sdb *subspace_ns = sdb_ns (test_ns, "subspace", true);
	sdb_set (subspace_ns, "some", "values", 0);
	sdb_set (subspace_ns, "are", "saved", 0);
	sdb_set (subspace_ns, "here", "lol", 0);
	return r;
}

bool test_sdb_diff_equal_empty() {
	Sdb *a = sdb_new0 ();
	Sdb *b = sdb_new0 ();
	mu_assert ("equal db (no diff)", sdb_diff (a, b, NULL));
	char *diff;
	mu_assert ("equal db (diff)", sdb_diff (a, b, &diff));
	mu_assert_streq (diff, "", "equal db diff");
	free (diff);
	sdb_free (a);
	sdb_free (b);
	mu_end;
}

bool test_sdb_diff_equal() {
	Sdb *a = test_sdb_new ();
	Sdb *b = test_sdb_new ();
	mu_assert ("equal db (no diff)", sdb_diff (a, b, NULL));
	char *diff;
	mu_assert ("equal db (diff)", sdb_diff (a, b, &diff));
	mu_assert_streq (diff, "", "equal db diff");
	free (diff);
	sdb_free (a);
	sdb_free (b);
	mu_end;
}

bool test_sdb_diff_ns_empty() {
	Sdb *a = test_sdb_new ();
	Sdb *b = test_sdb_new ();
	sdb_ns_unset (b, "emptyns", NULL);

	mu_assert ("empty ns removed (no diff)", !sdb_diff (a, b, NULL));
	char *diff;
	mu_assert ("empty ns removed (diff)", !sdb_diff (a, b, &diff));
	mu_assert_streq (diff, "-NS emptyns\n", "empty ns removed diff");
	free (diff);

	mu_assert ("empty ns added (no diff)", !sdb_diff (b, a, NULL));
	mu_assert ("empty ns added (diff)", !sdb_diff (b, a, &diff));
	mu_assert_streq (diff, "+NS emptyns\n", "empty ns added diff");
	free (diff);

	sdb_free (a);
	sdb_free (b);
	mu_end;
}

bool test_sdb_diff_ns() {
	Sdb *a = test_sdb_new ();
	Sdb *b = test_sdb_new ();
	sdb_ns_unset (b, "test", NULL);

	mu_assert ("ns removed (no diff)", !sdb_diff (a, b, NULL));
	char *diff;
	mu_assert ("ns removed (diff)", !sdb_diff (a, b, &diff));
	mu_assert_streq (diff,
		"-NS test\n"
		"-NS test/subspace\n"
   		"-   test/subspace/here=lol\n"
		"-   test/subspace/some=values\n"
		"-   test/subspace/are=saved\n"
		"-   test/a=123\n"
		"-   test/c=hello\n"
		"-   test/b=test\n", "ns removed diff");
	free (diff);

	mu_assert ("ns added (no diff)", !sdb_diff (b, a, NULL));
	mu_assert ("ns added (diff)", !sdb_diff (b, a, &diff));
	mu_assert_streq (diff,
		"+NS test\n"
		"+NS test/subspace\n"
		"+   test/subspace/here=lol\n"
		"+   test/subspace/some=values\n"
		"+   test/subspace/are=saved\n"
		"+   test/a=123\n"
		"+   test/c=hello\n"
		"+   test/b=test\n", "ns added diff");
	free (diff);

	sdb_free (a);
	sdb_free (b);
	mu_end;
}

bool test_sdb_diff_ns_sub() {
	Sdb *a = test_sdb_new ();
	Sdb *b = test_sdb_new ();
	sdb_ns_unset (sdb_ns (b, "test", 0), "subspace", NULL);

	mu_assert ("sub ns removed (no diff)", !sdb_diff (a, b, NULL));
	char *diff;
	mu_assert ("sub ns removed (diff)", !sdb_diff (a, b, &diff));
	mu_assert_streq (diff,
		"-NS test/subspace\n"
   		"-   test/subspace/here=lol\n"
		"-   test/subspace/some=values\n"
		"-   test/subspace/are=saved\n", "sub ns removed diff");
	free (diff);

	mu_assert ("sub ns added (no diff)", !sdb_diff (b, a, NULL));
	mu_assert ("sub ns added (diff)", !sdb_diff (b, a, &diff));
	mu_assert_streq (diff,
		"+NS test/subspace\n"
		"+   test/subspace/here=lol\n"
		"+   test/subspace/some=values\n"
		"+   test/subspace/are=saved\n", "sub ns added diff");
	free (diff);

	sdb_free (a);
	sdb_free (b);
	mu_end;
}

bool test_sdb_diff_kv() {
	Sdb *a = test_sdb_new ();
	Sdb *b = test_sdb_new ();
	sdb_unset (b, "more", 0);
	sdb_unset (sdb_ns (b, "test", false), "a", 0);

	mu_assert ("kv removed (no diff)", !sdb_diff (a, b, NULL));
	char *diff;
	mu_assert ("kv removed (diff)", !sdb_diff (a, b, &diff));
	mu_assert_streq (diff,
		"-   test/a=123\n"
  		"-   more=stuff\n", "ns removed diff");
	free (diff);

	mu_assert ("kv added (no diff)", !sdb_diff (b, a, NULL));
	mu_assert ("kv added (diff)", !sdb_diff (b, a, &diff));
	mu_assert_streq (diff,
		"+   test/a=123\n"
		"+   more=stuff\n", "ns added diff");
	free (diff);

	sdb_free (a);
	sdb_free (b);
	mu_end;
}

bool test_sdb_diff_kv_value() {
	Sdb *a = test_sdb_new ();
	Sdb *b = test_sdb_new ();
	sdb_set (b, "more", "cowbell", 0);
	sdb_set (sdb_ns (b, "test", false), "a", "reaper", 0);

	mu_assert ("kv value changed (no diff)", !sdb_diff (a, b, NULL));
	char *diff;
	mu_assert ("kv value changed (diff)", !sdb_diff (a, b, &diff));
	mu_assert_streq (diff,
		"-   test/a=123\n"
		"+   test/a=reaper\n"
		"-   more=stuff\n"
		"+   more=cowbell\n", "ns value changed diff");
	free (diff);

	sdb_free (a);
	sdb_free (b);
	mu_end;
}

int all_tests() {
	mu_run_test (test_sdb_diff_equal_empty);
	mu_run_test (test_sdb_diff_equal);
	mu_run_test (test_sdb_diff_ns_empty);
	mu_run_test (test_sdb_diff_ns);
	mu_run_test (test_sdb_diff_ns_sub);
	mu_run_test (test_sdb_diff_kv);
	mu_run_test (test_sdb_diff_kv_value);
	return tests_passed != tests_run;
}

int main(int argc, char **argv) {
	//Sdb *sdb = test_sdb_new();
	//sdb_query (sdb, "*");
	//sdb_query (sdb, "***");
	//sdb_free (sdb);
	//return 0;
	return all_tests();
}