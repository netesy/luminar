# Issues With Luminar

- This parses (12 > 3) and (12 > 13) but not  12 > 3 and 12 > 13  we are not generating the last comparison operator before the logical operator
- true and (false or true) leads generates a bad parsing which shows a precedence issue.
- There is a issue converting from boolean to int inside the vm
