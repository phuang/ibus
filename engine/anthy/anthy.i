/* anthy.i */
 %module anthy
 %{
 /* Put header files here or function declarations like below */
 #include <anthy/anthy.h>
 %}

%init %{
    anthy_init ();
%}


/* anthy_context_t */
%include anthy/anthy.h
struct anthy_context {};
%extend anthy_context {
    anthy_context () {
        return anthy_create_context ();
    }

    void reset () {
        anthy_reset_context (self);
    }

    int set_string (char *str) {
        return anthy_set_string (self, str);
    }

    void resize_segment (int a1, int a2) {
        anthy_resize_segment (self, a1, a2);
    }

    int get_stat (struct anthy_conv_stat *a1) {
        return anthy_get_stat (self, a1);
    }

    int get_segment_stat (int a1, struct anthy_segment_stat *a2) {
        return anthy_get_segment_stat (self, a1, a2);
    }

    int get_segment (int a1, int a2, char *a3, int a4) {
        return anthy_get_segment (self, a1, a2, a3, a4);
    }

    int commit_segment (int a1, int a2) {
        return anthy_commit_segment (self, a1, a2);
    }

    int set_prediction_string (const char *a1) {
        return anthy_set_prediction_string (self, a1);
    }
   
    int get_prediction_stat (struct anthy_prediction_stat *a1) {
        return anthy_get_prediction_stat (self, a1);
    }
    
    int get_prediction (int a1, char *a2, int a3) {
        return anthy_get_prediction (self, a1, a2, a3);
    }

    int commit_prediction (int a1) {
        return anthy_commit_prediction(self, a1);
    }

    void _print () {
        anthy_print_context (self);
    }
    
    int _set_encoding (int encoding) {
        return anthy_context_set_encoding (self, encoding);
    }

    int set_reconversion_mode (int mode) {
        return anthy_set_reconversion_mode (self, mode);
    }

    ~anthy_context () {
        anthy_release_context (self);
    }
};

