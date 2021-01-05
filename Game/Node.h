#pragma once
#include <vector>
#include "maths_funcs.h"

enum NodeType {
	NODE,
	MESH,
	LIGHT
};

class Node
{
	public:
		Node(mat4 matrix, NodeType nodeType);
		void setName(std::string name);
		std::string getName();
		void addChild(Node* child);
		int getNumberOfChildren();
		NodeType getNodeType();
		std::vector<Node*>& getChildren();
		mat4 getMatrix();
		void setMatrix(mat4 matrix);
		virtual void draw(mat4 matrix);

	protected:
		mat4 matrix;

	private:
		std::vector<Node*> children;
		std::string name;
		NodeType nodeType;
};
