**TESTING WITH BASIC COMMANDS**

**TESTING WITH REDIRECTION**

**TESTING WITH CD**

**TESTING WITH PIPES**

**TESTING WITH BATCHED COMMANDS**

**TESTING WITH SUBSHELLS**
process substitution is not handled.

1. echo "Start processing..." ; (cd txt ; cat phrases.txt | sort > subshell_sorted_phrases1.txt) ✅
2. echo "SUNBUN" ; (cd txt ; cat phrases.txt | sort > subshell_sorted_phrases2.txt) ; echo "FROSTY" ; date ; echo "BISKYY"✅
3. (cd txt; cat phrases.txt) > results/test1.txt ✅
4. (cd txt ; cat phrases.txt) | sort >> results/test2.txt ✅ (done twice to show append)
5. (cd txt ; cat phrases.txt) | sort | head ✅
6. echo "Start processing..." ; (cd txt ; cat phrases.txt | sort > subshell_sorted_phrases1.txt) ; (cd results ; uniq nested2.txt) >> txt/phrases_nested.txt ✅
7. (cd txt ; exit) ; echo "Hello" ✅

*Nested*
1. (cd txt ; (cat phrases.txt | sort > ../results/nested1.txt))✅
2. echo "FROSTY" ; (cd txt ; (cat phrases.txt | sort > ../results/nested2.txt)) ; echo "SNOWMAN" ✅ 
3. (cd results ; ((cat test1.txt | sort | head ) > ../txt/nestedFromResults.txt)) ✅

NOT WORKING :
1. wc <(cat txt/phrases.txt) ❌ --> process substitution


**TESTING GLOBBING WITH OTHER FUNCTIONALITIES**
1. (cd txt ; ls *.txt) ; echo "Frosty" ✅
2. (cd txt ; ls [p]*.txt) ; echo "Sunshine" ✅
3. cat txt/*.txt ✅
4. cat txt/*.txt > txt/everything.txt ✅
5. (cd txt ; cat [a-z]hrases?st**.txt) | sort ✅ // should only print sorted phrases_stats.txt
6. cat txt/phrases_[s]*.txt | sort ✅
7. cat txt/phrases_[s]*.txt | sort | uniq > txt/test_glob_pipe_redir.txt ✅
8. (cd glob ; ls t?.txt) > txt/glob_txt_files_single_digit.txt ✅
9. (cd glob ; ls t??.txt) >> txt/glob_txt_files_single_digit.txt ✅