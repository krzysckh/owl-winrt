(import
 (owl toplevel)
 (owl lazy)
 (owl sys)
 (lib))

(define (OK a b)
  (if (eqv? a b)
      (print "OK")
      (error "NOT OK" a)))

(λ (_)
  (OK (foo) 'bar)
  (OK (print "Hello, World!") #t)
  (OK (file? "Makefile") #t)

  0)
