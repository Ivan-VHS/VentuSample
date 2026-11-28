#!/usr/bin/env node
import { existsSync, readFileSync, realpathSync, statSync } from "node:fs";
import { dirname, isAbsolute, relative, resolve, sep } from "node:path";
import { fileURLToPath } from "node:url";

const ROOT = resolve(dirname(fileURLToPath(import.meta.url)), "..");
export const STAGES = ["Genesis","Foundation","Evolution","Ascension","Vanguard","Frontier","Dominion","Apex","Zenith","Singularity"];
const REQUIRED = new Map([
  [1,["scope"]],[2,["architecture","policy","build","ci"]],[3,["source","tests","runtime"]],
  [4,["integration","contract_tests","ci"]],[5,["target_build","resource_budget"]],
  [6,["platform_coverage","threat_model","security_tests","fuzz_or_fault_tests","accessibility"]],
  [7,["core_acceptance","compatibility","migration"]],[8,["release_candidate","provenance","recovery"]],
  [9,["support_policy","operations","release_regression","deprecation"]],
  [10,["reproducible_rebuild","audit_chain","independent_integration"]]
]);
const VERIFICATIONS = new Set(["documentation_only","source_reviewed","tests_passed","ci_passed","artifact_verified"]);

function checkPath(root,path,add) {
  if(typeof path!=="string"||!path||isAbsolute(path)||path.includes("\\")||path.split("/").some(part=>!part||part==="."||part==="..")){add("unsafe evidence path: "+String(path));return;}
  const absolute=resolve(root,path),rel=relative(root,absolute);
  if(rel===".."||rel.startsWith(".."+sep)){add("evidence path escapes repository: "+path);return;}
  try {
    if(!existsSync(absolute)){add("evidence path does not exist: "+path);return;}
    const info=statSync(absolute);
    if(!info.isFile()&&!info.isDirectory())add("evidence path is not a file or directory: "+path);
    const realRel=relative(root,realpathSync(absolute));
    if(realRel===".."||realRel.startsWith(".."+sep))add("evidence path resolves outside repository: "+path);
  } catch(error) { add("cannot inspect evidence path "+path+": "+error.message); }
}

export function validateProjectLadder(record,root=ROOT) {
  const errors=[],add=m=>errors.push(m);
  if(record?.schema!=="dak.project-maturity.v1"||record?.version!==1)add("unsupported project maturity schema or version");
  if(typeof record?.project!=="string"||!record.project.trim())add("project name is required");
  if(!/^Ivan-VHS\/[A-Za-z0-9._-]+$/.test(record?.repository??""))add("repository must be an Ivan-VHS owner/name");
  if(process.env.GITHUB_REPOSITORY&&record?.repository!==process.env.GITHUB_REPOSITORY)add("manifest repository does not match GITHUB_REPOSITORY");
  if(!Number.isInteger(record?.current_stage)||record.current_stage<1||record.current_stage>10)add("current_stage must be an integer from 1 through 10");
  if(record?.current_name!==STAGES[(record?.current_stage??0)-1])add("current_name does not match current_stage");
  if(!["provisional","verified"].includes(record?.assessment))add("assessment must be provisional or verified");
  if(!/^\d{4}-\d{2}-\d{2}$/.test(record?.assessment_date??""))add("assessment_date must use YYYY-MM-DD");
  if(typeof record?.assessment_basis!=="string"||record.assessment_basis.trim().length<20)add("assessment_basis must explain the assessment");
  if(record?.semver_independent!==true)add("semver_independent must remain true");

  const stages=record?.stages;
  if(!Array.isArray(stages)||stages.length!==10)add("exactly ten stage records are required");
  else {
    let highest=0,blocked=false;
    for(let i=0;i<10;i++){
      const stage=stages[i],number=i+1;
      if(stage?.number!==number||stage?.name!==STAGES[i]){add("stage "+number+" must be named "+STAGES[i]+" and remain in canonical order");continue;}
      if(!["met","pending","not_applicable"].includes(stage.status)){add(STAGES[i]+": invalid status");continue;}
      const evidence=Array.isArray(stage.evidence)?stage.evidence:[];
      if(stage.status==="met"){
        if(blocked)add(STAGES[i]+": a completed stage cannot follow a pending stage");
        if(!evidence.length)add(STAGES[i]+": met status requires evidence");
        const kinds=new Set();
        for(const item of evidence){
          if(typeof item?.kind==="string")kinds.add(item.kind);else add(STAGES[i]+": evidence kind is required");
          if(typeof item?.claim!=="string"||item.claim.trim().length<12)add(STAGES[i]+": evidence claim is too short");
          if(!VERIFICATIONS.has(item?.verification))add(STAGES[i]+": invalid verification type");
          checkPath(root,item?.path,add);
        }
        for(const kind of REQUIRED.get(number)??[])if(!kinds.has(kind))add(STAGES[i]+": missing required evidence kind "+kind);
        if(!blocked)highest=number;
      } else if(stage.status==="not_applicable"){
        if(blocked)add(STAGES[i]+": not_applicable cannot follow a pending stage");
        if(typeof stage.rationale!=="string"||stage.rationale.trim().length<20)add(STAGES[i]+": not_applicable requires a written rationale");
        for(const item of evidence)checkPath(root,item?.path,add);
        if(!blocked)highest=number;
      } else {
        blocked=true;
        for(const item of evidence)checkPath(root,item?.path,add);
      }
    }
    if(record?.current_stage!==highest)add("current_stage must equal the highest consecutive completed stage ("+highest+")");
  }
  const expectedNext=record?.current_stage===10?null:record?.current_stage+1;
  if(record?.next_stage!==expectedNext)add("next_stage must be the next cumulative stage or null at Singularity");
  if(expectedNext===null){if(!Array.isArray(record?.next_gate)||record.next_gate.length!==0)add("Singularity must have an empty next_gate");}
  else if(!Array.isArray(record?.next_gate)||!record.next_gate.length||record.next_gate.some(x=>typeof x!=="string"||x.trim().length<12))add("next_gate must contain specific acceptance work");
  if(record?.assessment==="verified"){
    const run=record?.verification_run;
    if(!run||!/^[0-9a-f]{40}$/i.test(run.commit??"")||typeof run.workflow!=="string"||!run.workflow||typeof run.environment!=="string"||!run.environment||typeof run.run_url!=="string"||!run.run_url.startsWith("https://"))add("verified assessment requires commit, workflow, environment and a CI run URL");
    const all=(stages??[]).flatMap(s=>s?.evidence??[]);
    if(!all.some(x=>x?.verification==="tests_passed"))add("verified assessment requires test evidence");
    if(!all.some(x=>x?.verification==="ci_passed"))add("verified assessment requires CI evidence");
    if((record?.current_stage??0)>=5&&!all.some(x=>x?.verification==="artifact_verified"))add("Vanguard or higher requires verified target artifacts");
  }
  return errors;
}

export function validateRepository(root=ROOT){
  const errors=[];
  try{
    const schema=JSON.parse(readFileSync(resolve(root,"schemas/project-ladder.v1.schema.json"),"utf8"));
    if(schema?.$id!=="https://raw.githubusercontent.com/Ivan-VHS/DAK/main/schemas/project-ladder.v1.schema.json")errors.push("project ladder schema $id is not canonical");
    if(schema?.properties?.schema?.const!=="dak.project-maturity.v1")errors.push("project ladder schema does not declare the canonical contract");
  }catch(error){errors.push("cannot read project ladder schema: "+error.message);}
  try{
    const record=JSON.parse(readFileSync(resolve(root,".github/project-ladder.v1.json"),"utf8"));
    errors.push(...validateProjectLadder(record,root));
  }catch(error){errors.push("cannot read project ladder manifest: "+error.message);}
  return errors;
}
if(process.argv[1]&&resolve(process.argv[1])===fileURLToPath(import.meta.url)){
  const errors=validateRepository();
  if(errors.length){for(const e of errors)process.stderr.write("ERROR: "+e+"\n");process.exitCode=1;}
  else process.stdout.write("Project maturity ladder: valid (cumulative gates and evidence paths checked)\n");
}
