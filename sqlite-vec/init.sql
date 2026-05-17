.load /home/xirro/sqlite-vec/vec0.so

create virtual table vec_chunks using vec0(
  id integer,
  news text,
  embedding float[8],
);

.read /home/xirro/sqlite-vec/vec_chunks_2048.sql

SELECT id, news, distance
FROM vec_chunks
WHERE embedding match
     '[0.70, 0.60, 0.65, 0.50, 0.30, 0.40, 0.75, 0.70]'
ORDER BY distance ASC
LIMIT 5;

UPDATE vec_chunks
SET 
  news = '新的新闻标题',
  embedding = '[0.70, 0.60, 0.65, 0.50, 0.30, 0.40, 0.75, 0.70]'
WHERE id = 123;

-- SELECT id, news
-- FROM vec_chunks
-- WHERE id = 123;

SELECT id, news, distance
FROM vec_chunks
WHERE embedding match
     '[0.70, 0.60, 0.65, 0.50, 0.30, 0.40, 0.75, 0.70]'
ORDER BY distance ASC
LIMIT 5;



DELETE FROM vec_chunks
WHERE id = 123; 

SELECT id, news, distance
FROM vec_chunks
WHERE embedding match
     '[0.70, 0.60, 0.65, 0.50, 0.30, 0.40, 0.75, 0.70]'
ORDER BY distance ASC
LIMIT 5;

DELETE FROM vec_chunks
WHERE news LIKE '%国产大模型能力提升245%';

SELECT id, news, distance
FROM vec_chunks
WHERE embedding match
     '[0.70, 0.60, 0.65, 0.50, 0.30, 0.40, 0.75, 0.70]'
ORDER BY distance ASC
LIMIT 5;