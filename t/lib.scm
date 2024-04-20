(define-library (lib)
  (import
   (owl toplevel))

  (export
   foo)

  (begin
    (define (foo)
      'bar)))
