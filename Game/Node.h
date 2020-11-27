#pragma once
#include <vector>
#include "maths_funcs.h"

enum NodeType {
	MESH
};

class Node
{
	public:
		Node(mat4 matrix, NodeType nodeType);
		void addChild(Node* child);
		int getNumberOfChildren();
		NodeType getNodeType();
		std::vector<Node*> getChildren();
		mat4 getMatrix();
		virtual void draw(mat4 matrix) = 0;

	protected:
		mat4 matrix;

	private:
		std::vector<Node*> children;
		NodeType nodeType;
};
