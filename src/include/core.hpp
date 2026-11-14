#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <memory>
#include "CJsonObject.hpp"
#include "random.hpp"

#pragma once

//这3个先保持硬编码
constexpr int k_peopleNum{64};
//列x
constexpr int k_columnNum{8};
//行y
constexpr int k_rowNum{8};

constexpr int k_defaultGender {2};
constexpr int k_defaultHeight {0};
constexpr int k_board_indexNotExist {-1};
constexpr int k_studentList_indexNotExist {-2};
constexpr int k_oJson_indexNotExist {-2};
constexpr int k_rulesList_indexNotExist {-3};
constexpr int k_idReserver {0};

inline const std::string json_EmptyRule {"{\"id\":0,\"diffGender\":0"};

inline const std::string keyStudents {"students"};
inline const std::string keyHeight {"height"};
inline const std::string keyGender {"gender"};
inline const std::string keyName {"name"};

inline const std::string keyRules {"rules"};
inline const std::string keyId {"id"};
inline const std::string keyIsInverse {"isInverse"};
inline const std::string keyDiffGender {"diffGender"};
inline const std::string keyL {"L"};
inline const std::string keyColumn {"columnRange"};
inline const std::string keyRow {"rowRange"};
inline const std::string keyC {"C"};
inline const std::string keyCircle {"circle"};

inline const std::string error_rangeNotCoincide {"范围无效\n"};
inline const std::string error_keyNotExist {"键不存在\n"};
inline const std::string error_setLineFailed {"行列范围设置失败\n"};
inline const std::string error_setCircleFailed {"圆形范围设置失败\n"};
inline const std::string error_studentJsonWrong {"学生信息错误\n"};
inline const std::string error_ruleJsonWrong {"规则信息错误\n"};

enum class Error
{
    rangeNotCoincide,
	keyNotExist,
    setLineFailed,
    setCircleFailed,
	studentJsonWrong,
	ruleJsonWrong,
};

struct Data
{
    int id {k_idReserver};
    int height {k_defaultHeight};
    int gender {k_defaultGender};
	std::string name {};
    bool isSelected {false};
};

struct Point
{
    int x {};
    int y {};
	
	bool operator== (const Point& p) const;
	bool operator!= (const Point& p) const;
};

class Range
{
public:
    Range();
    Range(int min, int max, bool isInverse);
	Range& operator=(const Range& range);
    
	bool set(neb::CJsonObject& oJson, bool isInverse);
    bool set(int min, int max, bool isInverse);
	bool setMin(int min);
	bool setMax(int max);
	void setIsInverse(bool isInverse);
	
	int getMin() const;
	int getMax() const;
	bool getIsInverse() const;
    bool isInRange(int num) const;

private:
    int m_min;
    int m_max;
    bool m_isInverse;
};

class CRange
{
public:
    CRange();
    CRange(Point p, int r, bool isInverse);
	CRange& operator=(const CRange& range);
    
	bool set(neb::CJsonObject& oJson, bool isInverse);
    bool set(Point centralPoint, int r, bool isInverse);
	bool setPoint(Point p);
	bool setRadius(int r);
	void setIsInverse(bool isInverse);
	
	Point getPoint() const;
	int getRadius() const;
	bool getIsInverse() const;
    bool isInRange(Point p) const;

private:
    Point m_p;
    int m_r;
    bool m_isInverse;
};

class StudentsList
{
public:
    StudentsList(std::string oJson);
	bool check();
	//排序
	void heightSort();
    void seatSort();
    void swap(int id1, int id2);
	void swapSeats(int index1, int index2);
    //操作 m_board:
	int getBoardId(Point point) const;
    Point getBoardPosition(int id) const;
    int getBoardIndex(int id) const;
    //操作 m_students:
	const std::string& getName(int id) const;
	int getGender(int id) const;
	int getHeight(int id) const;
	int getStudentsIndex(int id) const;
    //操作 m_oJson:
	int getJsonIndex(int id);
	const neb::CJsonObject& getJson() const;

public:
    bool addStudent(Data student);
    bool deleteStudent(int id);

private:
    neb::CJsonObject m_oJson;
    int m_oJsonLength;
    std::vector<Data> m_students;
    
    int m_index {-1};
    int m_peopleNum {0};
	int m_emptyId {-1};
    std::vector<int> m_board = std::vector<int> (k_peopleNum);
};

class Rule
{
public:
    Rule(neb::CJsonObject& rJson);
    Rule();
	
	Rule& operator=(const Rule& rule);
	
	// 基于 Json 的方法
	bool check(neb::CJsonObject& rJson, const StudentsList& students);
    bool set(neb::CJsonObject& rJson, const StudentsList& students);
    bool setL(neb::CJsonObject& rJson);
    bool setC(neb::CJsonObject& rJson, const StudentsList& students);
	
	// 基于 Range 的方法
	bool set(int id, bool allowDiffGender, StudentsList& students);
	bool setL(Range columnRange, Range rowRange);
	bool setC(CRange cRange, StudentsList& students);
	
	int getId() const;
	bool getL_IsInverse() const;
	bool getC_IsInverse() const;
	bool getL() const;
    bool getC() const;
	Range getColumnRange() const;
	Range getRowRange() const;
	int getCId() const;
	CRange getCRange() const;
	bool getAllow() const;
	
	bool isCoincide(const StudentsList& students) const;
	void refresh(StudentsList& students);
    void execute(StudentsList& students);

private:
    int m_id {};
    
    bool m_l;
	bool m_l_isInverse {false};
    Range m_columnRange {};
    Range m_rowRange {};
    
    bool m_c;
	bool m_c_isInverse {false};
	int m_cId;
    CRange m_CRange {};
    
    bool m_allowDiffGender;
};

class RulesList
{
public:
	RulesList(std::string rJson);
	bool check(neb::CJsonObject& rJson);
	bool set(const StudentsList& students);
	void execute(StudentsList& students);
	
	int getIndex(int id) const;
	Rule getRule(int index) const;
	const neb::CJsonObject& getJson() const;
	
	bool addRule(Rule& rule);
	bool deleteRule(int id);

private:
	neb::CJsonObject m_rJson;
	int m_rJsonLength;
	
	std::vector<Rule> m_rulesList;
};

class Main
{
public:
    Main(std::string oJson, std::string rJson);
    bool main();
	void execute();
	
	int getId(Point point) const;
    const std::string& getName(Point point) const;
	int getGender(Point point) const;
	int getHeight(Point point) const;
	void swapSeats(Point a, Point b);
	
	const neb::CJsonObject& getStudentsJson() const;
	const neb::CJsonObject& getRulesJson() const;
	
	bool addStudent(Data student);
	bool deleteStudent(int id);

private:
    void m_error(Error e);
    
    StudentsList m_students;
    RulesList m_rules;
	int cnt {};
};