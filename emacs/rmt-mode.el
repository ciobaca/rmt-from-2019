;; rmt-mode-el -- Major mode for RMT files

;; Author: Stefan Ciobaca
;; (C) 2016 Stefan Ciobaca stefan.ciobaca@gmail.com

(defvar rmt-mode-hook nil)

(defvar rmt-mode-map
  (let ((rmt-mode-map (make-keymap)))
    (define-key rmt-mode-map "\C-j" 'newline-and-indent)
    rmt-mode-map)
  "Keymap for RMT major mode")
(add-to-list 'auto-mode-alist '("\\.rmt\\'" . rmt-mode))

(setq rmt-keywords '("sorts" "subsort" "signature" "variables" "rewrite-system" "constrained-rewrite-system" "smt-narrow-search" "smt-unify" "smt-implies" "smt-satisfiability" "smt-prove" "show-equivalent" "with-base" "define" "by" "assert" "builtins" "compute""definedsearch" "for"))
(setq rmt-keywords-regexp (regexp-opt rmt-keywords 'words))

(setq rmt-types '("Int" "Bool"))
(setq rmt-types-regexp (regexp-opt rmt-types 'words))

(setq rmt-operators '(":" "->" "/" "=>" "<"))
(setq rmt-operators-regexp (regexp-opt rmt-operators))

(setq rmt-functions '("mplus" "mtimes" "mdiv" "mminus" "mle" "mgt" "mequals" "bimplies" "bequals" "bnot" "band" "bor"))
(setq rmt-functions-regexp (regexp-opt rmt-functions 'words))

(setq rmt-constants '("true" "false" "mzero" "mone" "mtwo" ))
(setq rmt-constants-regexp (regexp-opt rmt-constants 'words))

; generate optimized regexp for keywords
;(kill-new (regexp-opt '("sorts" "subsort" "signature" "rewrite") t))

(setq rmt-font-lock-keywords
      `(
        (,rmt-types-regexp . font-lock-type-face)
        (,rmt-constants-regexp . font-lock-constant-face)
        (,rmt-operators-regexp . font-lock-builtin-face)
        (,rmt-functions-regexp . font-lock-function-name-face)
        (,rmt-keywords-regexp . font-lock-keyword-face)
        ;; note: order above matters, because once colored, that part won't change.
        ;; in general, longer words first
        ))


(defvar rmt-mode-syntax-table
  (let ((rmt-mode-syntax-table (make-syntax-table)))
    (modify-syntax-entry ?/ ". 124b" rmt-mode-syntax-table)
    (modify-syntax-entry ?* ". 23" rmt-mode-syntax-table)
    (modify-syntax-entry ?\n "> b" rmt-mode-syntax-table)
    rmt-mode-syntax-table)
  "Syntax table for rmt-mode")

(define-derived-mode rmt-mode fundamental-mode
  (use-local-map rmt-mode-map)
  (set-syntax-table rmt-mode-syntax-table)
  (setq font-lock-defaults '((rmt-font-lock-keywords)))
  (setq major-mode 'rmt-mode)
  (setq mode-name "RMT"))

(provide 'rmt-mode)
