[EntityEditorProps(category: "GameScripted/Triggers", description: "Trigger that Spawns Environment or AI stuff.")]
class RLC_SpawnTriggerClass: SCR_BaseTriggerEntityClass
{
};

class RLC_SpawnTrigger : SCR_BaseTriggerEntity
{
	[Attribute("0", UIWidgets.CheckBox, "If checked, delete/Despawn when no player is in Trigger!")]
	protected bool m_bDelete;	
	[Attribute("0", UIWidgets.CheckBox, "Update Navmesh when spawning?")]
	protected bool updateNavmesh;	
	
	
	//[Attribute("0", UIWidgets.CheckBox, "If checked, generates the NavMesh for the env when spawning them.")]
	//protected bool m_bGenMeshOnSpawn;
	
	//future
	//[Attribute("0", UIWidgets.CheckBox, "Check if the AI spawns should be randomized!")]
	//protected bool m_bRandomizedSpawns;	
	
	//[Attribute("0", UIWidgets.Slider, "How many percentige of the AI spawns should be populated", "0 100 1")]
	//protected int PercentageAi;
	
	BaseGameMode GameMode;
	IEntity Owner;
	ArmaReforgerScripted GameSingleEntity;
	private RplComponent m_pRplComponent;
	protected int m_iCount = 0; // keep count of enemies
	
	
	ref array<IEntity> children = new array<IEntity>();
	ref array<RLC_AISpawnerComponent> childrenAiSpawner =  new array<RLC_AISpawnerComponent>;
	ref array<SCR_EnvSpawnerComponent> childrenEnvSpawner = new array<SCR_EnvSpawnerComponent>();
	
	override void OnInit(IEntity owner)
	{
		Owner = owner;
		super.OnInit(owner);
		if (!GetGame().InPlayMode())
            return;
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if(!m_pRplComponent) Debug.Error("RLC_SpawnTrigger cannot hook to the RplComponent please add one!");
		
		
		if(m_pRplComponent.IsMaster())
		
		
		GameSingleEntity =  GetGame();
		GameMode = GameSingleEntity.GetGameMode();
		RLC_Statics.GetAllChildren(this,children);
		//Get components we need
		foreach (int i, IEntity child : children)
		{	
			ref array<IEntity> childrenTmp = new array<IEntity>();
			auto ai = RLC_Statics.GetAiFromEnitity(child,childrenTmp);
			if(ai)
				childrenAiSpawner.Insert(ai);
			auto env = SCR_EnvSpawnerComponent.Cast(child.FindComponent(SCR_EnvSpawnerComponent));
			if(env)
				childrenEnvSpawner.Insert(env);
		}
		
		
	}
	

	
	//Thanks to Herbiie for his code and the wiki: https://github.com/Herbiie/ArmAReforgerMissionMakingGuide
    // Set up the filter
    override bool ScriptedEntityFilterForQuery(IEntity ent) {
        SCR_ChimeraCharacter cc = SCR_ChimeraCharacter.Cast(ent);
        if (!cc) return false; // If the entity is not a person, filter it out
        if (cc.GetFactionKey() != "USSR") return false; // If the entity does not have the Faction key of USSR, filter it out
        if (!IsAlive(cc)) return false; // If the entity is dead, filter it out
        return true; // Otherwise, include it!
    }
 
    override void OnActivate(IEntity ent)
    {
        ++m_iCount; // When activated (i.e. when an alive USSR soldier entity enters), add 1 to the number m_iCount
		if(m_iCount==1&&m_pRplComponent.IsMaster() && GameMode.IsLoaded()) Spawn(); // otherwise it should already be spawn
    }
 
    override void OnDeactivate(IEntity ent)
    {        
		
        --m_iCount; // When deactivated (i.e. if the soldier leaves or dies) take away 1 to the number m_iCount
		if(m_iCount==0 && m_pRplComponent.IsMaster()&&m_bDelete && GameMode.IsLoaded()) 
		{
			Despawn();
			//todo if master maybe delete?
		}
    }

	
	void Despawn()
	{
		if(!m_pRplComponent.IsMaster()) return;
		
		if(!children || children.Count()<=0) 
		{
			return; // nothing to spawn
		}
		
		foreach (int i, RLC_AISpawnerComponent ai : childrenAiSpawner)
		{	
			ai.RemoveSpawned();
		}		
		foreach (int i, SCR_EnvSpawnerComponent env : childrenEnvSpawner)
		{	
			env.RemoveSpawned();
		}
		//Future
		
	}	
	
	void Spawn()
	{
				
		if(!m_pRplComponent.IsMaster()) return;
		
		if(!children || children.Count()<=0) 
		{
			return; // nothing to spawn
		}
		
		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GameSingleEntity.GetAIWorld());
		foreach (int i, RLC_AISpawnerComponent ai : childrenAiSpawner)
		{	
			SpawnAi(ai);
		}		
		foreach (int i, SCR_EnvSpawnerComponent env : childrenEnvSpawner)
		{	
			SpawnEnv(env);
		}
				
		if (updateNavmesh && aiWorld)
		{
			foreach (int i, SCR_EnvSpawnerComponent env : childrenEnvSpawner)
			{	
				array<ref Tuple2<vector, vector>> areas = new array<ref Tuple2<vector, vector>>; //--- Min, max
				aiWorld.GetNavmeshRebuildAreas(env.GetSpawnedenviroment(), areas);
				GameSingleEntity.GetCallqueue().CallLater(aiWorld.RequestNavmeshRebuildAreas, 1000, false, areas); //--- Called *before* the entity is deleted with a delay, ensures the regeneration doesn't accidentaly get anything from the entity prior to full destruction
			}
		}

		
		
	}
	
	void SpawnAi(RLC_AISpawnerComponent ai)
	{
		ai.DoSpawnDefault();
	}
	
	void SpawnEnv(SCR_EnvSpawnerComponent env)
	{
		env.DoSpawnDefault();
		//Todo Gen navMesh
	}
	
 
}
