// mcpp lib root.
//
// Re-exports the three public module wrappers so a consumer can reach the whole
// library with one import:
//
//     import binary_tree;
//
// The individual wrappers remain public as well; this unit only names them in
// one place, which is what mcpp's [lib] root asks for.

export module binary_tree;

export import Yc.intrusive_binary_tree;
export import Yc.binary_tree;
export import Yc.parent_aware_binary_tree;
