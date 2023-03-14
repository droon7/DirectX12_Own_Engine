#include "pch.h"
#include "PmdBone.h"

using namespace::DirectX;

PmdBone::PmdBone()
{
}

//�{�[���e�[�u���A��]�s��AIK���A���[�V������������������
PmdBone::PmdBone(std::vector<PmdBoneData> pmdBoneDatas, std::vector<PMDIK> pmdIks, std::string motionPath)
{
	CreateBoneNodeTable(pmdBoneDatas);
	InitBoneMatrices(pmdBoneDatas);

	LoadPmdIks(pmdIks);
	vmdData = VMDData(motionPath);
}

//�e�q�֌W�̂���{�[���m�[�h�e�[�u�������
void PmdBone::CreateBoneNodeTable(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneNames.resize(pmdBoneDatas.size());
	boneNodeAddressArray.resize(pmdBoneDatas.size());

	//�{�[���l�[���Y�ƘA�z�z��ƃm�[�h�A���C�ɑS�{�[���f�[�^������
	for (int idx = 0; idx < pmdBoneDatas.size(); ++idx)
	{
		auto& pmdbonedata = pmdBoneDatas[idx];
		boneNames[idx] = pmdbonedata.boneName;
		if (boneNames[idx].find("�Ђ�") != std::string::npos)
		{
			kneeIdxes.emplace_back(idx);
		}

		auto& node = boneNodeTable[pmdbonedata.boneName];
		node.boneIdx = idx;
		node.startPos = pmdbonedata.pos;
		node.boneType = pmdbonedata.Type;
		node.ikParentBone = pmdbonedata.ikBoneNo;

		boneNodeAddressArray[idx] = &node;
	}

	//�S�Ă̐e�q�֌W�����
	for (auto& pmdbonedata : pmdBoneDatas)
	{
		//����continue
		if (pmdbonedata.parentNo >= pmdBoneDatas.size())
		{
			continue;
		}

		//�C�e���[�^���w���m�[�h�̐e�m�[�h���i�[���A������g���e�m�[�h�̎q�����o�Ɏ�����ǉ�����B
		//�e:�q�̊֌W��1:1-n�ƂȂ�A�t�͂��肦�Ȃ�
		auto parentName = boneNames[pmdbonedata.parentNo];
		boneNodeTable[parentName].children.emplace_back(&boneNodeTable[pmdbonedata.boneName]);
	}

}

void PmdBone::LoadPmdIks(std::vector<PMDIK> pmdIk)
{
	motionIKs.resize(pmdIk.size());

	for (int i = 0; i < pmdIk.size(); ++i)
	{
		motionIKs[i] = pmdIk[i];
	}
}

//�{�[���ϊ��s�񏉊���
void PmdBone::InitBoneMatrices(std::vector<PmdBoneData> pmdBoneDatas)
{
	boneMatrices.resize(pmdBoneDatas.size());

	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());


}

//�t���[�������ăL�[�t���[���𔭌��A�O�L�[�t���[���ƕ�Ԃ���]�s������肷��B
//TODO: ���̊֐��͕����������B
void PmdBone::SetBoneMatrices(unsigned int frameNo)
{
	std::fill(boneMatrices.begin(), boneMatrices.end(), XMMatrixIdentity());



	for (auto& bonemotion : vmdData.motionDatas)
	{
		//���_�̃{�[���f�[�^�ƃ��[�V�����̃{�[���f�[�^����v���邱�Ƃ��m�F�A�Ȃ����continue
		auto itBoneNode = boneNodeTable.find(bonemotion.first);
		if (itBoneNode == boneNodeTable.end())
		{
			continue;
		}

		auto node = boneNodeTable[bonemotion.first];
		auto motions = bonemotion.second;

		//�^����ꂽ�t���[���ԍ��ƃ��[�V�����̃t���[���ԍ��������Ă��邩���肷��֐��I�u�W�F�N�g
		auto predicate = [frameNo](const Motion& motion) {
			return motion.frameNo <= frameNo;
		};

		//�߂��l�̃C�e���[�^�[�𓾂�A�����ɕ���ł���f�[�^��T������̂Ń��o�[�X�C�e���[�^�[���g��
		auto rit = std::find_if(
			motions.rbegin(), motions.rend(), predicate
		);

		//���[�V�����f�[�^���Ȃ��Ȃ�continue
		if (rit == motions.rend())
		{
			continue;
		}

		//�O�L�[�t���[���Ǝ��L�[�t���[���ŕ�Ԃ��s��
		XMMATRIX rotation;
		auto it = rit.base();

		if (it == motions.end())
		{
			rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}
		else 
		{
			//�x�W�F�Ȑ����
			auto t = static_cast<float>(frameNo - rit->frameNo)
				/ static_cast<float>(it->frameNo - rit->frameNo);
			t = GetYFromXOnBezier(t, it->controlPoint1, it->controlPoint2, 12);
			rotation = XMMatrixRotationQuaternion(XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
			//rotation = XMMatrixRotationQuaternion(rit->quaternion);
		}

		//������]�s����i�[�B�Z���^�[����ċA�œ`��������B
		auto& pos = node.startPos;
		auto matrix = XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
			* rotation
			* XMMatrixTranslation(pos.x, pos.y, pos.z);
		boneMatrices[node.boneIdx] = matrix;
	}

	RecursiveMatrixMultiply(&boneNodeTable["�Z���^�["], XMMatrixIdentity());


}


void PmdBone::RecursiveMatrixMultiply(BoneNode* node, const XMMATRIX& mat)
{
	//�i�[����Ă���P�ʍs��ɓ����ϊ��s����|���A�f�[�^��ۑ�
	boneMatrices[node->boneIdx] *= mat;

	for (auto& childNode : node->children)
	{
		RecursiveMatrixMultiply(childNode, boneMatrices[node->boneIdx] );
	}
}

float PmdBone::GetYFromXOnBezier(float x, const XMFLOAT2& controlPoint1, const XMFLOAT2& controlPoint2, uint8_t max_steps)
{
	if (controlPoint1.x == controlPoint1.y && controlPoint2.x == controlPoint2.y)
	{
		return x;
	}

	float t = x;
	//�W��
	const float k0 = 1 + 3* controlPoint1.x - 3*controlPoint2.x;
	const float k1 = 3 * controlPoint2.x - 6 * controlPoint1.x;
	const float k2 = 3 * controlPoint1.x;

	//����l
	constexpr float epsilon = 0.0005f;

	//�ߎ��l�܂Ōv�Z����
	for (int i = 0; i < max_steps; ++i)
	{

		auto ft = k0*t*t*t + k1*t*t + k2*t - x;

		if (ft <= epsilon && ft >= -epsilon)
		{
			break;
		}
		t -= ft / 2;
	}

	auto r = 1 - t;
	auto ret = t*t*t + 3*t*t*r*controlPoint2.y + 3*t*r*r* controlPoint1.y;
	
	return ret;
}

void PmdBone::IKSolve()
{
	for (auto& ik : motionIKs)
	{
		auto childrenNodesCount = ik.nodeIdx.size();

		//CCD-IK�͏d���̂Ŋԓ_�����Ȃ���ΊȒP��IK�ŉ���
		switch (childrenNodesCount)
		{
		case 0: //�G���[
			assert(0);
			continue;
		case 1:
			SolveLookAt(ik);
			break;
		case 2:
			SolveCosineIK(ik);
			break;
		default:
			SolveCCDIK(ik);
		}
	}
}

void PmdBone::SolveCCDIK(const PMDIK& ik)
{
}

void PmdBone::SolveCosineIK(const PMDIK& ik)
{
	std::vector<XMVECTOR> positions;

	std::array<float, 2> edgeLens;

	//�^�[�Q�b�g
	auto& targetNode = boneNodeAddressArray[ik.boneIdx];
	auto targetPos = XMVector3Transform(XMLoadFloat3(&targetNode->startPos), boneMatrices[ik.boneIdx]);

	//���[�{�[��
	auto endNode = boneNodeAddressArray[ik.targetIdx];
	positions.emplace_back(XMLoadFloat3(&endNode->startPos));

	//���ԋy�у��[�g�{�[��
	for (auto& chainBoneIdx : ik.nodeIdx)
	{
		auto boneNode = boneNodeAddressArray[chainBoneIdx];
		positions.emplace_back(XMLoadFloat3(&boneNode->startPos));
	}

	//�킩��₷���̂��ߋt����
	reverse(positions.begin(), positions.end());

	edgeLens[0] = XMVector3Length(XMVectorSubtract(positions[1], positions[0])).m128_f32[0];
	edgeLens[1] = XMVector3Length(XMVectorSubtract(positions[2], positions[1])).m128_f32[0];

	//���[�g�{�[���̍��W�ϊ�
	positions[0] = XMVector3Transform(positions[0], boneMatrices[ik.nodeIdx[1]]);
	//��[�{�[���̍��W�ϊ�
	positions[2] = XMVector3Transform(positions[2], boneMatrices[ik.boneIdx]);

	auto linearVec = XMVectorSubtract(positions[2], positions[0]);
	float A = XMVector3Length(linearVec).m128_f32[0];
	float B = edgeLens[0];
	float C = edgeLens[1];

	linearVec = XMVector3Normalize(linearVec);
	
	//���[�g����^�񒆂ւ̊p�x�v�Z
	float theta1 = acosf((A * A + B * B - C * C) / (2 * A * B));
	float theta2 = acosf((B * B + C * C - A * A) / (2 * B * C));

	//��]���̐����A�^�񒆂̃m�[�h���G�m�[�h�Ȃ�x������]���Ƃ���
	XMVECTOR axis;
	if (std::find(kneeIdxes.begin(), kneeIdxes.end(), ik.nodeIdx[0]) == kneeIdxes.end())
	{
		auto vm = XMVector3Normalize(XMVectorSubtract(positions[2], positions[0]));
		auto vt = XMVector3Normalize(XMVectorSubtract(targetPos, positions[0]));
		axis = XMVector3Cross(vt, vm);
	}
	else
	{
		auto right = XMFLOAT3(1, 0, 0);
		axis = XMLoadFloat3(&right);
	}

	auto mat1 = XMMatrixTranslationFromVector(-positions[0]);
	mat1 *= XMMatrixRotationAxis(axis, theta1);
	mat1 *= XMMatrixTranslationFromVector(positions[0]);

	auto mat2 = XMMatrixTranslationFromVector(-positions[1]);
	mat2 *= XMMatrixRotationAxis(axis, theta2-XM_PI);
	mat2 *= XMMatrixTranslationFromVector(positions[1]);

	boneMatrices[ik.nodeIdx[1]] *= mat1;
	boneMatrices[ik.nodeIdx[0]] = mat2 * boneMatrices[ik.nodeIdx[1]];
	boneMatrices[ik.targetIdx] = boneMatrices[ik.nodeIdx[0]];
}

void PmdBone::SolveLookAt(const PMDIK& ik)
{
	//�ԓ_�͖������ߍŏ��̃m�[�h�����[�g�m�[�h�ƂȂ�
	auto rootNode = boneNodeAddressArray[ik.nodeIdx[0]];
	auto targetNode = boneNodeAddressArray[ik.boneIdx];

	auto rootPos1 = XMLoadFloat3(&rootNode->startPos);
	auto targetPos1 = XMLoadFloat3(&targetNode->startPos);

	auto rootPos2 = XMVector3TransformCoord(rootPos1, boneMatrices[ik.nodeIdx[0]]);
	auto targetPos2 = XMVector3TransformCoord(targetPos1, boneMatrices[ik.boneIdx]);

	auto originVec = XMVectorSubtract(targetPos1, rootPos1);
	auto targetVec = XMVectorSubtract(targetPos2, rootPos2);

	originVec = XMVector3Normalize(originVec);
	targetVec = XMVector3Normalize(targetVec);
	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 right = XMFLOAT3(1, 0, 0);

	boneMatrices[ik.nodeIdx[0]] = LookAtMatrix(originVec, targetVec, up, right);

}

//z����lookat�Ɍ�����]�s������
//lookat�ƕ⏕�x�N�g���œ��O�ς��s���A���W�n�����Ɛ��K�����ꂽx,y,z������������B������s��̊e��Ɋi�[����Ɖ�]����W��������B
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	//��������������
	XMVECTOR vz = XMVector3Normalize(lookat);

	//����y���x�N�g��
	XMVECTOR vy = XMVector3Normalize(XMLoadFloat3(&up));

	//x���x�N�g��
	XMVECTOR vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	//y���x�N�g��
	vy = XMVector3Normalize(XMVector3Cross(vz, vx));

	//lookat��up�̕����������ꍇright���g���쐬����
	if (std::abs(XMVector3Dot(vy, vz).m128_f32[0]) == 1.0f)
	{
		vx = XMVector3Normalize(XMLoadFloat3(&right));
		vy = XMVector3Normalize(XMVector3Cross(vz, vx));
		vx = XMVector3Normalize(XMVector3Cross(vy, vz));
	}

	XMMATRIX ret = XMMatrixIdentity();
	ret.r[0] = vx;
	ret.r[1] = vy;
	ret.r[2] = vz;

	return ret;
}

//z����C�ӂ̃x�N�g��(origin)�Ɍ�����s����v�Z�����̋t�s����v�Z����B�܂�origin��z���Ɍ��������]�s������(1)
//(1)��z����lookat�Ɍ��������]�s����|���Aorigin��lookat�Ɍ��������]�s������B
XMMATRIX PmdBone::LookAtMatrix(const XMVECTOR& origin, const XMVECTOR& lookat, XMFLOAT3& up, XMFLOAT3& right)
{
	return XMMatrixTranspose(LookAtMatrix(origin, up, right))
			* LookAtMatrix(lookat, up, right);
}
