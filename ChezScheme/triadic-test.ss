#! /usr/local/bin/scheme --program  # triadic-test.ss Copyright (c) 2022 Roger Turner

#| Based on TriadicMemory/Python/triadic_test.py, sdrsdm.py#randomSDR
  
Executable Chez Scheme program, eg:

  $ scheme --program triadic-test.ss x 1000 10
or
  $ chmod +x triadic-test.ss
  $ triadic-test.ss y 1000 10
  
(Creates triadic-test.so/.wpo/.wp, triadicmemory.so/.wpo files)
|#

#!chezscheme

(import (chezscheme)
        (triadicmemory))
        
(define test-len 100000)

(define SDR-SIZE 1000)
(define SDR-BITS 10)

;; SDR is ListOfFixnum [0..SDR-SIZE-1] length SDR-BITS

(define (run xyz sdr-size sdr-bits) ;; Symbol Integer Integer ->
  ;; create memory, store test-len triads, query and check specified x|y|z
  (for-each display `("Testing TriadicMemory with " ,test-len " SDRs of size "
                      ,sdr-size " and solidity of " ,sdr-bits " ON bits\n"))
  (let* (
      [mem  (make-triadic-memory sdr-size sdr-bits)]
      [t0   (cpu-time)]
      [X    (random-sdrs (+ test-len 2) sdr-size sdr-bits)]
      [t1   (cpu-time)]
      [_    (do ([k 0 (fx1+ k)]) ((fx=? k test-len))
              (store mem (vector-ref X k) (vector-ref X (fx1+ k)) (vector-ref X (fx+ k 2))))]
      [t2   (cpu-time)]
      [resp (make-vector test-len)]
      [_    (do ([k 0 (fx1+ k)]) ((fx=? k test-len))
              (vector-set! resp k
                (case xyz
                  [x (query mem '() (vector-ref X (fx1+ k)) (vector-ref X (fx+ k 2))) ]
                  [y (query mem (vector-ref X k) '() (vector-ref X (fx+ k 2))) ]
                  [z (query mem (vector-ref X k) (vector-ref X (fx1+ k)) '()) ])))]
      [t3   (cpu-time)]
      [count-errs
            (do ( [k 0 (fx1+ k)]
                  [k+ (case xyz [x 0] [y 1] [z 2]) (fx1+ k+)]
                  [errs 0 (if (equal? (vector-ref resp k) (vector-ref X k+)) errs
                              (fx1+ errs))])
                ((fx=? k test-len) errs)) ])
    (for-each display `(,test-len " vectors generated in " ,(fx- t1 t0) "ms\n"))    
    (for-each display `(,test-len " triples stored in " ,(fx- t2 t1) "ms\n"))
    (for-each display `(,test-len " queries in " ,(fx- t3 t2) "ms\n"))
    (for-each display `(,count-errs " errors\n"))))
  
(define (random-sdrs count N P) ;; Nat Nat Nat -> (Vector Count->SDR)
  ;; produce count random P-of-N SDRs
  (define (random-sdr)
    (let next-v ([sdr (list)])
      (cond
        [(fx=? (length sdr) P) (list-sort fx<? sdr) ]
        [else (let ([v (random N)])
                (if (memv v sdr)
                  (next-v sdr)
                  (next-v (cons v sdr)))) ])))
  (let ([ss (make-vector count)])
    (do ([k 0 (fx1+ k)]) ((fx=? k count) ss)
      (vector-set! ss k (random-sdr)))))

(define (compile/run args) ;; ListOfString ->
  ;; compile with "maximum speed" settings, run
  (if (and (pair? args) (string=? "run" (car args)))
    (run (string->symbol (cadr args))
         (string->number (caddr args))
         (string->number (cadddr args)))
  (parameterize (
      (library-directories '(("." . ".")))
      (compile-imported-libraries #t)
      (enable-cross-library-optimization #t)
      (optimize-level 3)
      (debug-level 0)
      (commonization-level 1)
      (generate-interrupt-trap #f)         ;; (only appropriate for library code)
      (generate-inspector-information #f)
      (generate-procedure-source-information #f)
      (#%$optimize-closures #t)
      (#%$track-dynamic-closure-counts #f)
      (cp0-effort-limit 20000)             ;; (default is 200)
      (cp0-score-limit   2000)             ;; (default is 20)
      #;(cp0-outer-unroll-limit 1)         ;; (default is 0) [1 slower]
      (generate-wpo-files #t)
      (undefined-variable-warnings #t)
      (generate-allocation-counts #f)
      (compile-file-message #f))
                                                                                            ;
    (compile-library "triadicmemory")
    (compile-program "triadic-test")
    (let ([missing
        (compile-whole-program "triadic-test.wpo" "triadic-test.wp")])
      (if (null? missing)
        (let ([command (string-append "scheme --program triadic-test.wp run "
                  (cond
                    [(null? args) (error 'triadic-test "specify x|y|z for query") ]
                    [(null? (cdr args))
                      (string-append (car args) " " (number->string SDR-SIZE)
                                                " " (number->string SDR-BITS)) ]
                    [(null? (cddr args))
                      (string-append (car args) " " (cadr args) " "
                        (number->string (fxdiv (string->number (cadr args)) 100))) ]
                    [else (string-append (car args) " " (cadr args) " " (caddr args)) ])) ])
          (system command)
          (exit)))
        (display missing)))))

(compile/run (command-line-arguments))
