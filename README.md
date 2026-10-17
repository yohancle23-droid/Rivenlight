# Rivenlight

Unreal Engine 5.8 (Blueprint 전용) 전투 기획 프로토타입입니다.

## 유료 에셋 (저장소에 포함되지 않음)

Fab 라이선스 때문에 아래 폴더는 `.gitignore`로 제외했습니다. 저장소를 받은 뒤 해당 에셋을 직접 가져와야 완전하게 동작합니다.

| 폴더 | 용도 | 비고 |
|---|---|---|
| `Content/BrutalDoubleAxeSet/` | 보스 탈리스 리본 모션/메시 | 이동 모션 `move_run_front` 에셋에서 "루트 강제 잠금"을 켜야 메시가 캡슐을 벗어나지 않음 |
| `Content/Katanas/` | 플레이어 카타나 (`SM_DaiKatana` 사용) | 5.8 호환 목록에 없어 5.2 프로젝트에 추가한 뒤 폴더를 복사해 사용 |
| `Content/Cyberman/` | 시도 후 보류한 플레이어 몸체 | 현재 사용하지 않음 |

## 플레이어 무기 소켓

- `SK_Mannequin`의 `hand_r` 아래에 `KatanaSocket`을 추가했습니다.
- `BP_ThirdPersonCharacter`의 `KatanaMesh`(스태틱 메시 컴포넌트)가 이 소켓에 붙습니다. 콜리전은 `NoCollision`입니다.