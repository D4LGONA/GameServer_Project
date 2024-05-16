#pragma once

// 상속의 기본형 
class Object
{
protected:
	int id;
	int x, y;
	int hp; // 체력
	int attack, defense; // 공격력, 방어력

public:
	Object();
	~Object();
};

