#include "core.hpp"

// Board 中所有不存在的id的index值都是-1
// StudentsList 中所有不存在的id的index值都是-2
// 所有不存在的index和point的id值都是0
// 所有空位置id小于0

// 身高是随便输入的,并非真实数据
// GUI 是 AI 写的,底层逻辑是我写的
// 拖拽功能还有一些问题尚未实现
// 规则有优先级
// Git

inline const std::string k_nameNotExist {"棍母"};

bool Point::operator== (const Point& p) const
{
	return (p.x == x && p.y == y);
}

bool Point::operator!= (const Point& p) const
{
	return (p.x != x || p.y != y);
}

Range::Range()
	:m_min {}
	,m_max {}
	,m_isInverse {false}
{
}

Range::Range(int min, int max, bool isInverse)
	:m_min {min}
	,m_max {max}
	,m_isInverse {isInverse}
{
}

Range& Range::operator=(const Range& range)
{
	if (this == &range)
	{
		return *this;
	}
	
	m_max = range.m_max;
	m_min = range.m_min;
	m_isInverse = range.m_isInverse;
	
	return *this;
}

bool Range::set(neb::CJsonObject& oJson, bool isInverse)
{
    if (oJson.GetArraySize() < 2)
	{
		return false;
	}
	
	int min {};
    int max {};
    
	oJson.Get(0, min);
    oJson.Get(1, max);
    
    return set(min, max, isInverse);
}

bool Range::set(int min, int max, bool isInverse)
{
    m_isInverse = isInverse;
    if (min <= max)
    {
        m_min = min;
        m_max = max;
		
        return true;
    }
    else
    {
        return false;
    }
}

bool Range::setMin(int min)
{
	if (min > m_max)
	{
		return false;
	}
	m_min = min;
	return true;
}

bool Range::setMax(int max)
{
	if (max < m_min)
	{
		return false;
	}
	m_max = max;
	return true;
}

void Range::setIsInverse(bool isInverse)
{
	m_isInverse = isInverse;
}

int Range::getMin() const
{
	return m_min;
}

int Range::getMax() const
{
	return m_max;
}

bool Range::getIsInverse() const
{
	return m_isInverse;
}

bool Range::isInRange(int num) const
{
    if (num >= m_min && num <= m_max)
	{
		return m_isInverse ? false : true;
	}   
    else
	{
		return m_isInverse ? true : false;
	}
}

CRange::CRange()
    :m_p {}
	,m_r {}
	,m_isInverse {false}
{
}

CRange::CRange(Point p, int r, bool isInverse)
    :m_p {p}
	,m_r {r}
	,m_isInverse {isInverse}
{
}

CRange& CRange::operator=(const CRange& range)
{
	if (this == &range)
	{
		return *this;
	}
	
	m_p = range.m_p;
	m_r = range.m_r;
	m_isInverse = range.m_isInverse;
	
	return *this;
}

bool CRange::set(neb::CJsonObject& oJson, bool isInverse)
{
    if (oJson.GetArraySize() < 3)
	{
		return false;
	}
	
	Point p {};
    int r {};
    
    oJson.Get(0, p.x);
    oJson.Get(1, p.y);
    oJson.Get(2, r);
    
    return set(p, r, isInverse);
}

bool CRange::set(Point p, int r, bool isInverse)
{
    if (r < 0)
    {
        return false;
    }
    
    m_p = p;
    m_r = r;
    m_isInverse = isInverse;
    
    return true;
}

bool CRange::setPoint(Point p)
{
	m_p = p;
	return true;
}

bool CRange::setRadius(int r)
{
	if (r < 0)
	{
		return false;
	}
	m_r = r;
	return true;
}

void CRange::setIsInverse(bool isInverse)
{
	m_isInverse = isInverse;
}

Point CRange::getPoint() const
{
	return m_p;
}

int CRange::getRadius() const
{
	return m_r;
}

bool CRange::getIsInverse() const
{
	return m_isInverse;
}

bool CRange::isInRange(Point p) const
{
    int xDelta {};
    int yDelta {};
    
    if (p.x <= m_p.x)
    {
        xDelta = m_p.x - p.x;
    }
    else
    {
        xDelta = p.x - m_p.x;
    }
    
    if (p.y <= m_p.y)
    {
        yDelta = m_p.y - p.y;
    }
    else
    {
        yDelta = p.y - m_p.y;
    }
    
    if (xDelta + yDelta <= m_r)
    {
 		return m_isInverse ? false : true;
    }
    else
    {
        return m_isInverse ? true : false;
    }
}

StudentsList::StudentsList(std::string oJson)
    :m_oJson (oJson)
    ,m_oJsonLength {m_oJson[keyStudents].GetArraySize()}
    ,m_students {(static_cast<size_t>(m_oJsonLength))}
{
    for (int i {}; i < m_oJsonLength; i++)
    {
        Data temp {};
        m_oJson[keyStudents][i].Get(keyId,temp.id);
        m_oJson[keyStudents][i].Get(keyHeight,temp.height);
        m_oJson[keyStudents][i].Get(keyGender,temp.gender);
        m_oJson[keyStudents][i].Get(keyName,temp.name);
        m_students[i] = temp;
    }
}

bool StudentsList::check()
{
    if (!m_oJson.KeyExist(keyStudents))
    {
        return false;
    }
    if (m_oJsonLength > k_peopleNum)
    {
        return false;
    }
	
	int maleNum {};
	int femaleNum {};
	
	for (int i {}; i < m_oJsonLength; i++)
    {
        if (!m_oJson[keyStudents][i].KeyExist(keyId))
		{
			return false;
		}
        else
		{
			if (m_students[i].id <= 0)
			{
				return false;
			}
			else
			{
				for (int j {}; j < i; j++)
				{
					if (m_students[j].id == m_students[i].id)
					{
						return false;
					}
				}
			}
		}
		
		if (!m_oJson[keyStudents][i].KeyExist(keyGender))
		{
			return false;
		}
		else
		{
			if (m_students[i].gender == 0)
			{
				maleNum++;
			}
			else if (m_students[i].gender == 1)
			{
				femaleNum++;
			}
			else
			{
				return false;
			}
		}
        
		// 以后可能不止 height 一个排序依据,同时可能不会局限于身高排序
		if (!m_oJson[keyStudents][i].KeyExist(keyHeight))
		{
			return false;
		}
        
		if (!m_oJson[keyStudents][i].KeyExist(keyName))
		{
			return false;
		}
    }
	
	if (m_oJsonLength + maleNum % 2 + femaleNum % 2 > k_peopleNum)
	{
		return false;
	}
	
    return true;
    
}

void StudentsList::heightSort()
{
    for (int i {}; i < m_oJsonLength; i++)
    {
        for (int j {i}; j < m_oJsonLength; j++)
        {
            if(m_students[j].height < m_students[i].height)
                std::swap(m_students[i],m_students[j]);
        }
    }
}

void StudentsList::seatSort()
{
    m_peopleNum = 0;
	m_index = -1;
	    
	while (m_peopleNum < m_oJsonLength && m_index + 1 < k_peopleNum)
    {
		bool isFound {false};
		int firstGender {k_defaultGender};
		int firstId {};
		
		for (int i {}; i < m_oJsonLength; i++)
        {
            if (!m_students[i].isSelected)
            {
                m_board[++m_index] = m_students[i].id;
                firstGender = m_students[i].gender;
				firstId = i;
                
                m_peopleNum++;
                m_students[i].isSelected = true;
                				
                break;
            }
        }
        
		// 这里下标不从0开始
		for (int i {firstId}; i < m_oJsonLength; i++)
        {
            if (!m_students[i].isSelected && m_students[i].gender == firstGender)
            {
                m_board[++m_index] = m_students[i].id;
                
                m_peopleNum++;
                m_students[i].isSelected = true;
                
                isFound = true;
                				
                break;
            }
        }
        if (!isFound && firstGender != k_defaultGender)
		{
			m_board[++m_index] = m_emptyId--;
        }
    }
	
	for (Data& student : m_students)
	{
		student.isSelected = false;
	}
}

void StudentsList::swap(int id1, int id2)
{
    int index1 {getBoardIndex(id1)};
    int index2 {getBoardIndex(id2)};
	
	swapSeats(index1, index2);
}

void StudentsList::swapSeats(int index1, int index2)
{
	if (index1 == k_board_indexNotExist || index2 == k_board_indexNotExist)
	{
		return;
	}
    
    std::swap(m_board[index1], m_board[index2]);
}

int StudentsList::getBoardId(Point point) const
{
    int index {(point.y - 1) * k_columnNum + (point.x - 1)};
    if (index < 0 || index > m_index)
    {
        return k_idReserver;
    }
    return m_board[index];
}

Point StudentsList::getBoardPosition(int id) const
{
    int index {getBoardIndex(id)};
	if (index == k_board_indexNotExist)
	{
		return Point {0,0};
	}
    
	index++;
    Point point{};
    
    int mod {index % k_columnNum};
    
    if (mod == 0)
    {
        point.x = k_columnNum;
        point.y = index / k_columnNum;
    }
    else
    {
        point.x = mod;
        point.y = (index - mod) / k_columnNum + 1;
    }
    
    return point;
}

int StudentsList::getBoardIndex(int id) const
{
    if (id == k_idReserver)
	{
		return k_board_indexNotExist;
	}
	
	for(int i {}; i <= m_index; i++)
    {
        if (m_board[i] == id)
        {
            return i;
        }
    }
    
    return k_board_indexNotExist;
}

const std::string& StudentsList::getName(int id) const
{
    int index {getStudentsIndex(id)};
    
    if (index == k_studentList_indexNotExist)
	{
		return k_nameNotExist;
	}
    
    return m_students[index].name;
}

int StudentsList::getGender(int id) const
{
    int index {getStudentsIndex(id)};
	
    if (index == k_studentList_indexNotExist)
	{
		return k_defaultGender;
	}
	
    return {m_students[index].gender};
}

int StudentsList::getHeight(int id) const
{
    int index {getStudentsIndex(id)};
	
    if (index == k_studentList_indexNotExist)
	{
		return k_defaultHeight;
	}
	
    return {m_students[index].height};
}

int StudentsList::getStudentsIndex(int id) const
{
	//id值小于0是不存在
    if (id <= 0)
        return k_studentList_indexNotExist;
    
    for (int i {}; i < m_oJsonLength; i++)
    {
        if (m_students[i].id == id)
            {
                return i;
            }
    }
	
	return k_studentList_indexNotExist;
}

int StudentsList::getJsonIndex(int id)
{
    int temp {};
    for (int i {}; i < m_oJsonLength; i++)
    {
        m_oJson[keyStudents][i].Get(keyId, temp);
        
        if (temp == id)
        {
            return i;
        }
    }
    
    return k_oJson_indexNotExist;
}

const neb::CJsonObject& StudentsList::getJson() const
{
	return m_oJson;
}

bool StudentsList::addStudent(Data student)
{
    if (student.id == k_idReserver)
	{
		return false;
	}
	
	// 人满了就不添
	if (k_peopleNum < m_oJsonLength + 1)
	{
		return false;
	}
	
	int index {getStudentsIndex(student.id)};
    
    // id不存在就添加
    if (index == k_studentList_indexNotExist)
    {
        m_students.push_back(student);
        
        neb::CJsonObject newStudent;
        newStudent.Add(keyId, student.id);
        newStudent.Add(keyHeight, student.height);
        newStudent.Add(keyGender, student.gender);
		newStudent.Add(keyName, student.name);
        
        m_oJson[keyStudents].Add(newStudent);
        m_oJsonLength++;
        
    }
    // id已经存在就修改
    else
    {
        int jsonIndex {getJsonIndex(student.id)};
        if (jsonIndex == k_oJson_indexNotExist)
        {
            return false;
        }
        m_oJson[keyStudents][jsonIndex].Replace(keyHeight, student.height);
        m_oJson[keyStudents][jsonIndex].Replace(keyGender, student.gender);
        m_oJson[keyStudents][jsonIndex].Replace(keyName, student.name);
		m_students[index] = student;
    }
    
    return true;
}
bool StudentsList::deleteStudent(int id)
{
    if (id == k_idReserver)
	{
		return false;
	}
	
	int students_index {getStudentsIndex(id)};
    // 只能删除已存在的id
    if (students_index == k_studentList_indexNotExist)
    {
        return false;
    }
    else
    {
        std::swap(m_students[students_index], m_students.back());
        m_students.pop_back();
        
        neb::CJsonObject newJson;
        newJson.AddEmptySubArray(keyStudents);
        int jsonIndex {getJsonIndex(id)};
        
        for (int i {}; i < m_oJsonLength; i++)
        {
            if (i == jsonIndex)
            {
                continue;
            }
            
            newJson[keyStudents].Add(m_oJson[keyStudents][i]);
        }
        
        m_oJson = std::move(newJson);
        m_oJsonLength = m_oJson[keyStudents].GetArraySize();
    }
	
	int board_index {getBoardIndex(id)};
	if (board_index == k_board_indexNotExist)
	{
		return false;
	}
	else
	{
		m_board[board_index] = m_emptyId--;
	}
    
	return true;
}

RulesList::RulesList(std::string rJson)
	:m_rJson {rJson}
	,m_rJsonLength {m_rJson[keyRules].GetArraySize()}
	,m_rulesList (static_cast<size_t>(m_rJsonLength))
{
}

bool RulesList::check(neb::CJsonObject& rJson)
{
	if (!rJson.KeyExist(keyRules))
    {
        return false;
    }
	
	for (int i {}; i < m_rJsonLength; ++i)
	{
		int id1 {};
		rJson[keyRules][i].Get(keyId, id1);
		
		for (int j {i + 1}; j < m_rJsonLength; ++j)
		{
			int id2 {};
			rJson[keyRules][j].Get(keyId, id2);
			
			if (id1 == id2)
			{
				return false;
			}
		}
	}
	
	return true;
}

bool RulesList::set(const StudentsList& students)
{
	if (!check(m_rJson))
    {
        return false;
    }
    
	for (int i {}; i < m_rJsonLength; ++i)
	{
		if (!m_rulesList[i].check(m_rJson[keyRules][i], students))
		{
			return false;
		}
		if (!m_rulesList[i].set(m_rJson[keyRules][i], students))
		{
			continue; // 这里要做些什么
		}
		
	}
	
	return true;
}

void RulesList::execute(StudentsList& students)
{
	for (int i {m_rJsonLength - 1}; i >= 0; --i)
	{
		m_rulesList[i].refresh(students);
		if (!m_rulesList[i].isCoincide(students))
		{
			continue;
		}
		m_rulesList[i].execute(students);
	}
}

int RulesList::getIndex(int id) const
{
	int index {k_rulesList_indexNotExist};
	
	for (int i {}; i < m_rJsonLength; ++i)
	{
		if (id == m_rulesList[i].getId())
		{
			index = i;
			break;
		}
	}
	
	return index;
}

Rule RulesList::getRule(int index) const
{
	if (index < 0 || index > m_rJsonLength)
	{
		return Rule ();
	}
	
	return m_rulesList[index];
}

const neb::CJsonObject& RulesList::getJson() const
{
	return m_rJson;
}

bool RulesList::addRule(Rule& rule)
{
	int id {rule.getId()};
	if (id == k_idReserver)
	{
		return false;
	}
	int index {getIndex(id)};
	
	neb::CJsonObject newRule {};
	newRule.Add(keyId, id);
	newRule.Add(keyDiffGender, rule.getAllow());
	
	if (rule.getL())
	{
		newRule.AddEmptySubArray(keyL);
		
		neb::CJsonObject newRule_L {};
		newRule_L.Add(keyIsInverse, rule.getL_IsInverse());
		newRule_L.AddEmptySubArray(keyColumn);
		newRule_L[keyColumn].Add(rule.getColumnRange().getMin());
		newRule_L[keyColumn].Add(rule.getColumnRange().getMax());
		newRule_L.AddEmptySubArray(keyRow);
		newRule_L[keyRow].Add(rule.getRowRange().getMin());
		newRule_L[keyRow].Add(rule.getRowRange().getMax());
		
		newRule[keyL].Add(newRule_L);
	}
	
	if (rule.getC())
	{
		newRule.AddEmptySubArray(keyC);
		
		neb::CJsonObject newRule_C {};
		newRule_C.Add(keyIsInverse, rule.getC_IsInverse());
		newRule_C.AddEmptySubArray(keyCircle);
		newRule_C[keyCircle].Add(rule.getCId());
		newRule_C[keyCircle].Add(rule.getCRange().getRadius());
		
		newRule[keyC].Add(newRule_C);
	}
	
	if (index == k_rulesList_indexNotExist)
	{
		m_rulesList.push_back(rule);
		m_rJson[keyRules].Add(newRule);
		++m_rJsonLength;
	}
	else
	{
		m_rulesList[index] = rule;
		m_rJson[keyRules].Replace(index, newRule);
	}
	
	return true;
}

bool RulesList::deleteRule(int id)
{
	if (id == k_idReserver)
	{
		return false;
	}
	int index {getIndex(id)};
	if (index == k_rulesList_indexNotExist)
	{
		return false;
	}
	
	m_rulesList.erase(m_rulesList.begin() + index);
	neb::CJsonObject newJson {};
	newJson.AddEmptySubArray(keyRules);
	for (int i {}; i < m_rJsonLength; ++i)
	{
		if (i == index)
		{
			continue;
		}
		newJson[keyRules].Add(m_rJson[keyRules][i]);
	}
	m_rJson = std::move(newJson);
	--m_rJsonLength;
	
	return true;
}

Rule::Rule(neb::CJsonObject& rJson)
    :m_l {rJson.KeyExist(keyL)}
    ,m_c {rJson.KeyExist(keyC)}
	,m_cId {}
	// 无需在意这个奇怪的构造
    ,m_allowDiffGender {rJson.KeyExist(keyDiffGender)}
{
    rJson.Get(keyId, m_id);
    
    if (m_allowDiffGender)
    {
        rJson.Get(keyDiffGender, m_allowDiffGender);
    }
    else
    {
        m_allowDiffGender = false;
    }
}

Rule::Rule()
	:m_id {}
	,m_l {false}
	,m_c {false}
	,m_cId {}
	,m_allowDiffGender {false}
{
}

Rule& Rule::operator=(const Rule& rule)
{
	if (this == &rule)
	{
		return *this;
	}
	
	m_id = rule.m_id;
    m_l = rule.m_l;
	m_l_isInverse = rule.m_l_isInverse;
    m_columnRange = rule.m_columnRange;
    m_rowRange = rule.m_rowRange;
    m_c = rule.m_c;
	m_c_isInverse = rule.m_c_isInverse;
	m_cId = rule.m_cId;
    m_CRange = rule.m_CRange;
    m_allowDiffGender = rule.m_allowDiffGender;
	
	return *this;
}

bool Rule::check(neb::CJsonObject& rJson, const StudentsList& students)
{
	if (!rJson.KeyExist(keyId))
	{
		return false;
	}
	else
	{
		int id {};
		rJson.Get(keyId, id);
		if (students.getStudentsIndex(id) == k_studentList_indexNotExist)
		{
			return false;
		}
	}
	
	bool l {rJson.KeyExist(keyL)};
	bool c {rJson.KeyExist(keyC)};
	
	if (!l && !c)
	{
		return false;
	}
	
	if (l)
	{
		if (rJson[keyL].GetArraySize() < 1)
		{
			return false;
		}
		if (!rJson[keyL][0].KeyExist(keyIsInverse))
		{
			return false;
		}
	}
	if (c)
	{
		if (rJson[keyC].GetArraySize() < 1)
		{
			return false;
		}
		if (!rJson[keyC][0].KeyExist(keyIsInverse))
		{
			return false;
		}
	}
	
	return true;
}

bool Rule::set(neb::CJsonObject& rJson, const StudentsList& students)
{	
	rJson.Get(keyId, m_id);
	rJson.Get(keyDiffGender, m_allowDiffGender);
	m_l = rJson.KeyExist(keyL);
	m_c = rJson.KeyExist(keyC);
	
	if (m_l)
	{
		if (!setL(rJson[keyL][0]))
		{
			return false;
		}
	}
	if (m_c)
	{
		if (!setC(rJson[keyC][0], students))
		{
			return false;
		}
	}
	
	return true;
}

bool Rule::setL(neb::CJsonObject& rJson)
{
    rJson.Get(keyIsInverse, m_l_isInverse);
    
    if (rJson.KeyExist(keyColumn))
    {
        if(!m_columnRange.set(rJson[keyColumn], m_l_isInverse))
        {
            return false;
        }
    }
    else
    {
        m_columnRange.set(1, k_columnNum, m_l_isInverse);
    }
    
    if (rJson.KeyExist(keyRow))
    {
        if(!m_rowRange.set(rJson[keyRow], m_l_isInverse))
        {
            return false;
        }
    }
    else
    {
        m_rowRange.set(1, k_rowNum, m_l_isInverse);
    }
    
    return true;
}

bool Rule::setC(neb::CJsonObject& rJson, const StudentsList& students)
{
    rJson.Get(keyIsInverse, m_c_isInverse);
    
    if (rJson.KeyExist(keyCircle))
    {
        if (rJson[keyCircle].GetArraySize() < 2)
		{
			return false;
		}
		
		rJson[keyCircle].Get(0, m_cId);
		int r {};
		rJson[keyCircle].Get(1, r);
		
		Point p {students.getBoardPosition(m_cId)};
		if (p == Point {0, 0})
		{
			return false;
		}
		
		if(!m_CRange.set(p, r, m_c_isInverse))
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    
    return true;
}

bool Rule::set(int id, bool allowDiffGender, StudentsList& students)
{
	if (students.getStudentsIndex(id) == k_studentList_indexNotExist)
	{
		return false;
	}
	m_id = id;
	m_allowDiffGender = allowDiffGender;
	return true;
}

bool Rule::setL(Range columnRange, Range rowRange)
{
	if (columnRange.getIsInverse() != rowRange.getIsInverse())
	{
		return false;
	}
	
	m_l = true;
	m_l_isInverse = columnRange.getIsInverse();
	m_columnRange = columnRange;
	m_rowRange = rowRange;
	
	return true;
}

bool Rule::setC(CRange cRange, StudentsList& students)
{
	m_cId = students.getBoardId(cRange.getPoint());
	if (m_cId == k_idReserver)
	{
		return false;
	}
	m_c = true;
	m_c_isInverse = cRange.getIsInverse();
	m_CRange = cRange;
	
	return true;
}

int Rule::getId() const
{
	return m_id;
}

bool Rule::getL_IsInverse() const
{
	return m_l_isInverse;
}

bool Rule::getC_IsInverse() const
{
	return m_c_isInverse;
}

bool Rule::getL() const
{
    return m_l;
}

bool Rule::getC() const
{
    return m_c;
}

Range Rule::getColumnRange() const
{
	return m_columnRange;
}

Range Rule::getRowRange() const
{
	return m_rowRange;
}

int Rule::getCId() const
{
	return m_cId;
}

CRange Rule::getCRange() const
{
	return m_CRange;
}

bool Rule::getAllow() const
{
	return m_allowDiffGender;
}

bool Rule::isCoincide(const StudentsList& students) const
{
	// C和L不是同时存在时无需判断
	if (!(m_l && m_c))
	{
		return true;
	}
	
	int cnt {};
	for (int x {1}; x <= k_columnNum; ++x)
	{
		for (int y {1}; y <= k_rowNum; ++y)
		{
			// 空位不作数
			if (students.getBoardId(Point {x, y}) == k_idReserver)
			{
				continue;
			}
			if (m_columnRange.isInRange(x) && m_rowRange.isInRange(y) && m_CRange.isInRange(Point {x, y}))
			{
				++cnt;
			}
		}
	}
	
	if (cnt > 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Rule::refresh(StudentsList& students)
{
	if (students.getStudentsIndex(m_id) == k_studentList_indexNotExist)
	{
		m_l = false;
		m_c = false;
	}
	if (students.getStudentsIndex(m_cId) == k_studentList_indexNotExist)
	{
		m_c = false;
	}
	else
	{
		m_CRange.setPoint(students.getBoardPosition(m_cId));
	}
}

void Rule::execute(StudentsList& students)
{
	Point point {};
    int tempId {};
    bool isFound {false};
	
    for (int tryNum {}; tryNum < 1000; ++tryNum)
    {
		point.x = Random::getRandom(1, k_columnNum);
        point.y = Random::getRandom(1, k_rowNum);
        tempId = students.getBoardId(point);
		
		if (tempId <= k_idReserver)
		{
			continue;
		}
        
        bool ruleL {};
        if (m_l)
        {
            ruleL = m_columnRange.isInRange(point.x) && m_rowRange.isInRange(point.y);
        }
        else
        {
            ruleL = true;
        }
        
        bool ruleC {};
        if (m_c)
        {
            ruleC = m_CRange.isInRange(point);
        }
        else
        {
            ruleC = true;
        }
        
        bool ruleGender {};
        if (m_allowDiffGender) //允许异性为true直接通过
        {
            ruleGender = true;
        }
        else
        {
            //同性为true
            ruleGender = students.getGender(tempId) == students.getGender(m_id);
        }
		
		//if (!ruleGender) {std::cout << "有问题\n";} else {std::cout << "无问题\n";}
		
        if (ruleL && ruleC && ruleGender)
		{
			isFound = true;
			break;
		}
    }
    
	if (!isFound)
	{
		return;
	}
	students.swap(tempId, m_id);
}
    

Main::Main(std::string oJson, std::string rJson)
:m_students (oJson)
,m_rules (rJson)
{
	if (m_students.check())
	{
		m_students.heightSort();
		m_students.seatSort();
	}
	else
	{
		m_error(Error::studentJsonWrong);
	}
}

bool Main::main()
{            
    ++cnt;
	if (cnt == 1)
	{
		if (!m_rules.set(m_students))
		{
			return false;
		}
	}
	m_rules.execute(m_students);
    return true;
}

void Main::execute()
{
	m_rules.execute(m_students);
}

int Main::getId(Point point) const
{
    return m_students.getBoardId(point);
}

const std::string& Main::getName(Point point) const
{
    int id {m_students.getBoardId(point)};
    return m_students.getName(id);
}

int Main::getGender(Point point) const
{
	int id {m_students.getBoardId(point)};
	return m_students.getGender(id);
}

int Main::getHeight(Point point) const
{
	int id {m_students.getBoardId(point)};
	return m_students.getHeight(id);
}

void Main::swapSeats(Point a, Point b)
{
	int index1 {m_students.getBoardIndex(m_students.getBoardId(a))};
	int index2 {m_students.getBoardIndex(m_students.getBoardId(b))};
	m_students.swapSeats(index1, index2);
}

const neb::CJsonObject& Main::getStudentsJson() const
{
	return m_students.getJson();
}

const neb::CJsonObject& Main::getRulesJson() const
{
	return m_rules.getJson();
}

bool Main::addStudent(Data student)
{
	return m_students.addStudent(student);
}

bool Main::deleteStudent(int id)
{
	return m_students.deleteStudent(id);
}

void Main::m_error(Error e)
{
    switch(e)
    {
        case Error::rangeNotCoincide:
            std::cout << error_rangeNotCoincide;
            break;
		case Error::keyNotExist:
            std::cout << error_keyNotExist;
            break;
        case Error::setLineFailed:
            std::cout << error_setLineFailed;
            break;
        case Error::setCircleFailed:
            std::cout << error_setCircleFailed;
            break;
		case Error::studentJsonWrong:
            std::cout << error_studentJsonWrong;
            break;
		case Error::ruleJsonWrong:
            std::cout << error_ruleJsonWrong;
            break;
    }
}