(let ((read (lambda (prompt) (ffi:call "builtins.input" (prompt))))
      (print (lambda (out) (ffi:call "builtins.print" (out))))
      (str->int (lambda (str) (ffi:call "builtins.int" (str))))
      (+ (lambda (lhs rhs) (ffi:call "operator.add" (lhs rhs))))
      (- (lambda (lhs rhs) (ffi:call "operator.sub" (lhs rhs))))
      (* (lambda (lhs rhs) (ffi:call "operator.mul" (lhs rhs))))
      (/ (lambda (lhs rhs) (ffi:call "operator.div" (lhs rhs))))
      (% (lambda (lhs rhs) (ffi:call "operator.mod" (lhs rhs)))))

  (let ((lhs (str->int (read "lhs?")))
	(rhs (str->int (read "rhs?"))))
    
    (print (+ lhs rhs))))
