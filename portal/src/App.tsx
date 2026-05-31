import { useEffect, useState } from "react";
import ReactMarkdown from "react-markdown";
import { Link, NavLink, Route, Routes, useParams } from "react-router-dom";

type ReleaseManifest = {
  version: string;
  summary: string;
  notes_url: string;
  firmware_url: string;
  published_at: string;
  sha256: string;
  size: number;
};

type ReleaseEntry = {
  version: string;
  title: string;
  published_at: string;
  summary: string;
  notes_path: string;
  notes_url: string;
  firmware_url: string;
};

type ReleaseIndex = {
  releases: ReleaseEntry[];
};

const releaseDataBase = `${import.meta.env.BASE_URL}release-data`;

async function fetchJson<T>(path: string): Promise<T> {
  const response = await fetch(path, { cache: "no-store" });
  if (!response.ok) {
    throw new Error(`Request failed for ${path}`);
  }
  return response.json() as Promise<T>;
}

function formatDate(value: string) {
  if (!value) {
    return "Pending date";
  }
  const date = new Date(value);
  if (Number.isNaN(date.getTime())) {
    return value;
  }
  return date.toLocaleDateString(undefined, {
    year: "numeric",
    month: "long",
    day: "numeric",
  });
}

function useReleaseData() {
  const [currentRelease, setCurrentRelease] = useState<ReleaseManifest | null>(
    null,
  );
  const [releaseIndex, setReleaseIndex] = useState<ReleaseEntry[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState("");

  useEffect(() => {
    let cancelled = false;

    async function load() {
      try {
        setLoading(true);
        const [current, index] = await Promise.all([
          fetchJson<ReleaseManifest>(`${releaseDataBase}/current.json`),
          fetchJson<ReleaseIndex>(`${releaseDataBase}/releases/index.json`),
        ]);
        if (!cancelled) {
          setCurrentRelease(current);
          setReleaseIndex(index.releases ?? []);
          setError("");
        }
      } catch (loadError) {
        if (!cancelled) {
          setError(
            loadError instanceof Error
              ? loadError.message
              : "Unable to load release data.",
          );
        }
      } finally {
        if (!cancelled) {
          setLoading(false);
        }
      }
    }

    load();
    return () => {
      cancelled = true;
    };
  }, []);

  return { currentRelease, releaseIndex, loading, error };
}

function App() {
  const releaseData = useReleaseData();

  return (
    <div className="shell">
      <header className="masthead">
        <div className="brand-block">
          <div className="eyebrow">SnowLeopard firmware delivery</div>
          <h1>GitHub-hosted releases, notes, and OTA metadata in one place.</h1>
          <p className="lede">
            This portal is the public release surface for SnowLeopard. The
            device checks the same file-based metadata that is rendered here, so
            the release website and OTA manifest stay aligned.
          </p>
        </div>
        <nav className="topnav" aria-label="Primary">
          <NavLink to="/">Overview</NavLink>
          <NavLink to="/releases">Releases</NavLink>
          <a
            href="https://github.com/ThingEngineer/SnowLeopard"
            target="_blank"
            rel="noopener noreferrer"
          >
            Repository
          </a>
        </nav>
      </header>

      <main className="content">
        <Routes>
          <Route path="/" element={<OverviewPage {...releaseData} />} />
          <Route path="/releases" element={<ReleasesPage {...releaseData} />} />
          <Route
            path="/releases/:version"
            element={<ReleaseNotesPage {...releaseData} />}
          />
        </Routes>
      </main>
    </div>
  );
}

function OverviewPage({
  currentRelease,
  releaseIndex,
  loading,
  error,
}: {
  currentRelease: ReleaseManifest | null;
  releaseIndex: ReleaseEntry[];
  loading: boolean;
  error: string;
}) {
  const newest = releaseIndex[0];

  return (
    <>
      <section className="hero-panel">
        <div className="metric-card accent">
          <span className="metric-label">Current manifest version</span>
          <strong>{currentRelease?.version ?? "loading"}</strong>
          <span>
            {currentRelease?.summary ??
              "Release metadata will appear here once loaded."}
          </span>
        </div>
        <div className="metric-card">
          <span className="metric-label">Release notes</span>
          <strong>{newest?.title ?? "Waiting for release data"}</strong>
          <span>
            {newest
              ? formatDate(newest.published_at)
              : "Publish your first release note file."}
          </span>
        </div>
        <div className="metric-card">
          <span className="metric-label">OTA artifact</span>
          <strong>
            {currentRelease?.firmware_url ? "Configured" : "Pending"}
          </strong>
          <span>
            {currentRelease?.firmware_url ||
              "Point current.json at your GitHub Release binary asset."}
          </span>
        </div>
      </section>

      <section className="section-grid">
        <article className="panel">
          <h2>How OTA works</h2>
          <p>
            SnowLeopard firmware checks <code>release-data/current.json</code>.
            When the version in that file is newer than the installed firmware,
            the device Settings page can show update available and offer the OTA
            action.
          </p>
          <p>
            Publish firmware binaries as GitHub Release assets, keep release
            notes in Markdown files, and update the manifest with the new
            version, binary URL, checksum, and size.
          </p>
        </article>

        <article className="panel">
          <h2>Release checklist</h2>
          <ol>
            <li>
              Update <code>include/firmware_version.h</code>.
            </li>
            <li>
              Update <code>release-data/current.json</code>.
            </li>
            <li>
              Add the matching Markdown file in{" "}
              <code>release-data/releases/</code>.
            </li>
            <li>
              Create and push a <code>v*</code> tag so the firmware release
              workflow publishes the binary.
            </li>
            <li>
              Push the manifest and notes changes so GitHub Pages reflects the
              new release.
            </li>
          </ol>
        </article>
      </section>

      <section className="panel release-spotlight">
        <div className="panel-head">
          <h2>Latest release</h2>
          {newest ? (
            <Link to={`/releases/${newest.version}`}>Read full notes</Link>
          ) : null}
        </div>
        {loading ? <p>Loading release data...</p> : null}
        {error ? <p className="error-text">{error}</p> : null}
        {!loading && !error && newest ? (
          <>
            <div className="release-meta">
              <span>{newest.version}</span>
              <span>{formatDate(newest.published_at)}</span>
            </div>
            <p>{newest.summary}</p>
          </>
        ) : null}
      </section>
    </>
  );
}

function ReleasesPage({
  releaseIndex,
  loading,
  error,
}: {
  releaseIndex: ReleaseEntry[];
  loading: boolean;
  error: string;
}) {
  return (
    <section className="panel">
      <div className="panel-head">
        <h2>Release archive</h2>
        <span>{releaseIndex.length} releases tracked</span>
      </div>
      {loading ? <p>Loading release archive...</p> : null}
      {error ? <p className="error-text">{error}</p> : null}
      {!loading && !error ? (
        <div className="release-list">
          {releaseIndex.map((release) => (
            <article className="release-row" key={release.version}>
              <div>
                <div className="release-meta">
                  <span>{release.version}</span>
                  <span>{formatDate(release.published_at)}</span>
                </div>
                <h3>{release.title}</h3>
                <p>{release.summary}</p>
              </div>
              <div className="release-actions">
                <Link to={`/releases/${release.version}`}>View notes</Link>
                <a
                  href={release.firmware_url}
                  target="_blank"
                  rel="noopener noreferrer"
                >
                  Download firmware
                </a>
              </div>
            </article>
          ))}
        </div>
      ) : null}
    </section>
  );
}

function ReleaseNotesPage({
  releaseIndex,
  loading,
  error,
}: {
  releaseIndex: ReleaseEntry[];
  loading: boolean;
  error: string;
}) {
  const { version } = useParams();
  const release = releaseIndex.find((entry) => entry.version === version);
  const [markdown, setMarkdown] = useState("");
  const [notesError, setNotesError] = useState("");

  useEffect(() => {
    let cancelled = false;

    async function loadMarkdown() {
      if (!release?.notes_path) {
        setMarkdown("");
        return;
      }
      try {
        const response = await fetch(
          `${releaseDataBase}/${release.notes_path}`,
          { cache: "no-store" },
        );
        if (!response.ok) {
          throw new Error("Unable to load release notes.");
        }
        const content = await response.text();
        if (!cancelled) {
          setMarkdown(content);
          setNotesError("");
        }
      } catch (loadError) {
        if (!cancelled) {
          setNotesError(
            loadError instanceof Error
              ? loadError.message
              : "Unable to load release notes.",
          );
        }
      }
    }

    loadMarkdown();
    return () => {
      cancelled = true;
    };
  }, [release]);

  return (
    <section className="panel notes-panel">
      <div className="panel-head">
        <div>
          <h2>{release?.title ?? version ?? "Release notes"}</h2>
          {release ? (
            <div className="release-meta">
              <span>{release.version}</span>
              <span>{formatDate(release.published_at)}</span>
            </div>
          ) : null}
        </div>
        <Link to="/releases">Back to archive</Link>
      </div>
      {loading ? <p>Loading release archive...</p> : null}
      {error ? <p className="error-text">{error}</p> : null}
      {!loading && !error && !release ? (
        <p className="error-text">
          Release {version} was not found in the index.
        </p>
      ) : null}
      {notesError ? <p className="error-text">{notesError}</p> : null}
      {markdown ? (
        <div className="markdown-body">
          <ReactMarkdown>{markdown}</ReactMarkdown>
        </div>
      ) : null}
    </section>
  );
}

export default App;
