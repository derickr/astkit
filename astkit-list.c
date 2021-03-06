#include "astkit.h"

zend_class_entry* astkit_list_ce = NULL;

/* {{{ proto void AstKitList::__construct() */
static PHP_METHOD(AstKitList, __construct) {
	zval *thz = getThis();
	astkit_object* objval = ASTKIT_FETCH_OBJ(thz);
	zend_arena* old_arena = CG(ast_arena);

	CG(ast_arena) = zend_arena_create(32 * 1024);
	objval->node = zend_ast_create_list(0, ZEND_AST_STMT_LIST);
	objval->tree = emalloc(sizeof(astkit_tree));
	objval->tree->root = objval->node;
	objval->tree->arena = CG(ast_arena);
	objval->tree->refcount = 1;

	CG(ast_arena) = old_arena;
	zend_hash_index_add(&ASTKITG(cache), (zend_ulong)objval->node, thz);
} /* }}} */

/* {{{ proto int AstKitList::numChildren() */
static PHP_METHOD(AstKitList, numChildren) {
	astkit_object* objval = ASTKIT_FETCH_OBJ(getThis());
	zend_ast_list* node = (zend_ast_list*)objval->node;
	RETURN_LONG(node->children);
} /* }}} */

/* {{{ proto object AstKitList::getChild(int child[, bool zval_as_value = true]) */
ZEND_BEGIN_ARG_INFO(AstKitList_getChild_arginfo, 0)
	ZEND_ARG_INFO(0, child)
	ZEND_ARG_INFO(0, zval_as_value)
ZEND_END_ARG_INFO()
static PHP_METHOD(AstKitList, getChild) {
	astkit_object* objval = ASTKIT_FETCH_OBJ(getThis());
	zend_ast_list* listNode = (zend_ast_list*)objval->node;
	zend_long child;
	zend_bool zval_as_value = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|b", &child, &zval_as_value) == FAILURE) {
		return;
	}

	if ((child < 0) || (child >= ((zend_long)(listNode->children)))) {
		php_error_docref(NULL, E_WARNING,
		  "Invalid child " ZEND_LONG_FMT ", current node has %d children",
		  child, listNode->children);
		return;
	}
	astkit_create_object(return_value, listNode->child[child], objval->tree, zval_as_value);
} /* }}} */

/* {{{ proto void AstKitList::apend(AstKit|mixed $node) */
ZEND_BEGIN_ARG_INFO(AstKitList_append_arginfo, 0)
	ZEND_ARG_INFO(0, node)
ZEND_END_ARG_INFO()
static PHP_METHOD(AstKitList, append) {
	astkit_object* objval = ASTKIT_FETCH_OBJ(getThis());
	zend_ast_list* listNode = (zend_ast_list*)objval->node;
	zval *node;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &node) == FAILURE) {
		return;
	}

	astkit_graft(objval, listNode->children, node);
} /* }}} */


static zend_function_entry astkit_list_methods[] = {
	PHP_ME(AstKitList, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	PHP_ME(AstKitList, numChildren, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(AstKitList, getChild, AstKitList_getChild_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(AstKitList, append, AstKitList_append_arginfo, ZEND_ACC_PUBLIC)
	PHP_FE_END
};

int astkit_list_minit(INIT_FUNC_ARGS) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "AstKitList", astkit_list_methods);
	astkit_list_ce = zend_register_internal_class_ex(&ce, astkit_node_ce);

	return SUCCESS;
}
