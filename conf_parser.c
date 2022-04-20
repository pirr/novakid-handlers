#include <yaml.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include "conf_parse.h"

void print_escaped(yaml_char_t * str, size_t length);
int usage(int ret);

struct conf_node {
    char* tag;
    char* value;
    struct conf_node* left;
    struct conf_node* right;
};

int parse(char *path_to_conf)
{
    FILE *conf;
    if ((conf = fopen(path_to_conf, "rb")) == NULL) {
        fprintf(stderr, "%s: failed to open %s (%d %s)\n",
                path_to_conf, path_to_conf, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    yaml_parser_t parser;
    yaml_event_t event;
    int flow = -1; /** default no flow style collections */

    if (!yaml_parser_initialize(&parser))
        fprintf(stderr, "Failed to initialize parser!\n");
    
    yaml_parser_set_input_file(&parser, conf);

    while (true) {
        yaml_event_type_t type;
        
        if (!yaml_parser_parse(&parser, &event)) {
            if ( parser.problem_mark.line || parser.problem_mark.column ) {
                fprintf(stderr, "Parse error: %s\nLine: %lu Column: %lu\n",
                    parser.problem,
                    (unsigned long)parser.problem_mark.line + 1,
                    (unsigned long)parser.problem_mark.column + 1);
            }
            else {
                fprintf(stderr, "Parse error: %s\n", parser.problem);
            }
            exit(EXIT_FAILURE);
        }
        type = event.type;

        switch (type)
        {
            case YAML_NO_EVENT:
                printf("-DOC");
                if (!event.data.document_end.implicit)
                    printf(" ...");
                break;
            case YAML_STREAM_START_EVENT:
                printf("+STR\n");
            case YAML_STREAM_END_EVENT:
                printf("-STR\n");
                break;
            case YAML_DOCUMENT_START_EVENT:
                printf("+DOC");
                if (!event.data.document_start.implicit)
                    printf(" ---");
                printf("\n");
                break;
            case YAML_DOCUMENT_END_EVENT:
                printf("-DOC");
                if (!event.data.document_end.implicit)
                    printf(" ...");
                printf("\n");
                break;
            case YAML_MAPPING_START_EVENT:
                printf("+MAP");
                if (flow == 0 && event.data.mapping_start.style == YAML_FLOW_MAPPING_STYLE)
                    printf(" {}");
                else if (flow == 1)
                    printf(" {}");
                if (event.data.mapping_start.anchor)
                    printf(" &%s", event.data.mapping_start.anchor);
                if (event.data.mapping_start.tag)
                    printf(" <%s>", event.data.mapping_start.tag);
                printf("\n");
                break;
            case YAML_MAPPING_END_EVENT:
                printf("-MAP\n");
                break;
            case YAML_SEQUENCE_START_EVENT:
                printf("+SEQ");
                if (flow == 0 && event.data.sequence_start.style == YAML_FLOW_SEQUENCE_STYLE)
                    printf(" []");
                else if (flow == 1)
                    printf(" []");
                if (event.data.sequence_start.anchor)
                    printf(" &%s", event.data.sequence_start.anchor);
                if (event.data.sequence_start.tag)
                    printf(" <%s>", event.data.sequence_start.tag);
                printf("\n");
                break;
            case YAML_SEQUENCE_END_EVENT:
                printf("-SEQ\n");
                break;
            case YAML_SCALAR_EVENT:
                printf("=VAL");
                if (event.data.scalar.anchor)
                    printf(" &%s", event.data.scalar.anchor);
                if (event.data.scalar.tag)
                    printf(" <%s>", event.data.scalar.tag);
                switch (event.data.scalar.style) {
                case YAML_PLAIN_SCALAR_STYLE:
                    printf(" :");
                    break;
                case YAML_SINGLE_QUOTED_SCALAR_STYLE:
                    printf(" '");
                    break;
                case YAML_DOUBLE_QUOTED_SCALAR_STYLE:
                    printf(" \"");
                    break;
                case YAML_LITERAL_SCALAR_STYLE:
                    printf(" |");
                    break;
                case YAML_FOLDED_SCALAR_STYLE:
                    printf(" >");
                    break;
                case YAML_ANY_SCALAR_STYLE:
                    abort();
                }
                print_escaped(event.data.scalar.value, event.data.scalar.length);
                printf("\n");
            case YAML_ALIAS_EVENT:
                printf("=ALI *%s\n", event.data.alias.anchor);
                break;
            default:
                abort();
                break;
            
            yaml_event_delete(&event);
            
        }

        if (type == YAML_STREAM_END_EVENT)
            break;
    }

    assert(!fclose(conf));
    yaml_parser_delete(&parser);
    fflush(stdout);

    exit(EXIT_SUCCESS);
}

void print_escaped(yaml_char_t * str, size_t length)
{
    int i;
    char c;

    for (i = 0; i < length; i++) {
        c = *(str + i);
        if (c == '\\')
            printf("\\\\");
        else if (c == '\0')
            printf("\\0");
        else if (c == '\b')
            printf("\\b");
        else if (c == '\n')
            printf("\\n");
        else if (c == '\r')
            printf("\\r");
        else if (c == '\t')
            printf("\\t");
        else
            printf("%c", c);
    }
}

int usage(int ret) {
    fprintf(stderr, "Usage: libyaml-parser [--flow (on|off|keep)] [<input-file>]\n");
    return ret;
}
